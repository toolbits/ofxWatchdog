/*
**      IridiumFrameworks
**
**      Original Copyright (C) 2013 - 2013 HORIGUCHI Junshi.
**                                          http://iridium.jp/
**                                          zap00365@nifty.com
**      Portions Copyright (C) <year> <author>
**                                          <website>
**                                          <e-mail>
**      Version     openFrameworks
**      Website     http://iridium.jp/
**      E-mail      zap00365@nifty.com
**
**      This source code is for Xcode.
**      Xcode 4.6.2 (LLVM compiler 4.2)
**
**      ofxWatchdog.cpp
**
**      ------------------------------------------------------------------------
**
**      The MIT License (MIT)
**
**      Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
**      associated documentation files (the "Software"), to deal in the Software without restriction,
**      including without limitation the rights to use, copy, modify, merge, publish, distribute,
**      sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
**      furnished to do so, subject to the following conditions:
**      The above copyright notice and this permission notice shall be included in all copies or
**      substantial portions of the Software.
**      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
**      BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
**      IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
**      WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
**      OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
**      以下に定める条件に従い、本ソフトウェアおよび関連文書のファイル（以下「ソフトウェア」）の複製を
**      取得するすべての人に対し、ソフトウェアを無制限に扱うことを無償で許可します。
**      これには、ソフトウェアの複製を使用、複写、変更、結合、掲載、頒布、サブライセンス、および、または販売する権利、
**      およびソフトウェアを提供する相手に同じことを許可する権利も無制限に含まれます。
**      上記の著作権表示および本許諾表示を、ソフトウェアのすべての複製または重要な部分に記載するものとします。
**      ソフトウェアは「現状のまま」で、明示であるか暗黙であるかを問わず、何らの保証もなく提供されます。
**      ここでいう保証とは、商品性、特定の目的への適合性、および権利非侵害についての保証も含みますが、それに限定されるものではありません。
**      作者または著作権者は、契約行為、不法行為、またはそれ以外であろうと、ソフトウェアに起因または関連し、
**      あるいはソフトウェアの使用またはその他の扱いによって生じる一切の請求、損害、その他の義務について何らの責任も負わないものとします。
*/

#include "ofxWatchdog.h"

ofxWatchdog ofxWatchdog::_singleton;
static char g_stack[SIGSTKSZ];
static sig_atomic_t volatile g_pid;
static sig_atomic_t volatile g_pfd;
static sig_atomic_t volatile g_override;
static sig_atomic_t volatile g_verbose;

ofxWatchdog::ofxWatchdog(void)
{
    initialize();
}

ofxWatchdog::~ofxWatchdog(void)
{
    terminate();
}

bool ofxWatchdog::watch(int msec, bool reboot, bool override, bool verbose)
{
    stack_t stack;
    int pfd[2];
    int code;
    bool result(false);
    
    g_override = override;
    g_verbose = verbose;
    if (atexit(&onExit) == 0) {
        stack.ss_sp = &g_stack;
        stack.ss_size = sizeof(g_stack);
        stack.ss_flags = 0;
        if (sigaltstack(&stack, NULL) == 0) {
            while (true) {
                if (pipe(pfd) == 0) {
                    g_pid = fork();
                    if (g_pid > 0) {
                        close(pfd[1]);
                        g_pfd = pfd[0];
                        if (fcntl(g_pfd, F_SETFL, O_NONBLOCK) == 0) {
                            if (parent(msec, &code) && reboot) {
                                terminate();
                                initialize();
                                continue;
                            }
                            else {
                                exit(code);
                            }
                        }
                        else {
                            error("ofxWatchdog [parent] controling pipe failed.");
                        }
                    }
                    else if (g_pid == 0) {
                        close(pfd[0]);
                        g_pfd = pfd[1];
                        if (fcntl(g_pfd, F_SETFL, O_NONBLOCK) == 0) {
                            if (child()) {
                                result = true;
                            }
                        }
                        else {
                            error("ofxWatchdog [child] controling pipe failed.");
                        }
                    }
                    else {
                        close(pfd[0]);
                        close(pfd[1]);
                        error("ofxWatchdog [parent] forking failed.");
                    }
                }
                else {
                    error("ofxWatchdog [parent] allocating pipe failed.");
                }
                break;
            }
        }
        else {
            error("ofxWatchdog [parent] allocating stack failed.");
        }
    }
    else {
        error("ofxWatchdog [parent] setting exit handler failed.");
    }
    return result;
}

void ofxWatchdog::clear(void)
{
    static char const beacon = '.';
    
    if (g_pid == 0) {
        write(g_pfd, &beacon, sizeof(beacon));
    }
    return;
}

void ofxWatchdog::onExit(void)
{
    terminate();
    if (g_pid == 0) {
        _exit(EXIT_SUCCESS);
    }
    return;
}

void ofxWatchdog::initialize(void)
{
    g_pid = -1;
    g_pfd = -1;
    return;
}

void ofxWatchdog::terminate(void)
{
    if (g_pfd >= 0) {
        close(g_pfd);
        g_pfd = -1;
    }
    return;
}

bool ofxWatchdog::parent(int msec, int* code)
{
    char temp[256];
    int hangup;
    int pid;
    int status;
    bool signal;
    char beacon;
    bool result(false);
    
    *code = EXIT_SUCCESS;
    hangup = 0;
    while (true) {
        if ((pid = waitpid(g_pid, &status, WNOHANG)) > 0) {
            if (WIFEXITED(status)) {
                if ((*code = WEXITSTATUS(status)) == EXIT_SUCCESS) {
                    log("ofxWatchdog [parent] detects exit.");
                }
                else {
                    snprintf(temp, sizeof(temp), "ofxWatchdog [parent] detects error exit : %d", *code);
                    log(temp);
                    result = true;
                }
            }
            else if (WIFSIGNALED(status)) {
                *code = EXIT_FAILURE;
                result = true;
                switch (WTERMSIG(status)) {
                    case SIGTRAP:
                        log("ofxWatchdog [parent] detects SIGTRAP, process shutdown.");
                        result = false;
                        break;
                    case SIGILL:
                        log("ofxWatchdog [parent] detects SIGILL.");
                        break;
                    case SIGABRT:
                        log("ofxWatchdog [parent] detects SIGABRT.");
                        break;
                    case SIGFPE:
                        log("ofxWatchdog [parent] detects SIGFPE.");
                        break;
                    case SIGBUS:
                        log("ofxWatchdog [parent] detects SIGBUS.");
                        break;
                    case SIGSEGV:
                        log("ofxWatchdog [parent] detects SIGSEGV.");
                        break;
                    default:
                        snprintf(temp, sizeof(temp), "ofxWatchdog [parent] detects unknown signal : %d", WTERMSIG(status));
                        log(temp);
                        break;
                }
            }
            else {
                pid = 0;
            }
        }
        if (pid == 0) {
            signal = true;
            if (read(g_pfd, &beacon, sizeof(beacon)) <= 0) {
                beacon = 'X';
            }
            switch (beacon) {
                case '.':
                    hangup = 0;
                    signal = false;
                    break;
                case 'I':
                    log("ofxWatchdog [parent] detects SIGILL.");
                    break;
                case 'A':
                    log("ofxWatchdog [parent] detects SIGABRT.");
                    break;
                case 'F':
                    log("ofxWatchdog [parent] detects SIGFPE.");
                    break;
                case 'B':
                    log("ofxWatchdog [parent] detects SIGBUS.");
                    break;
                case 'S':
                    log("ofxWatchdog [parent] detects SIGSEGV.");
                    break;
                default:
                    if (++hangup < msec) {
                        signal = false;
                    }
                    else {
                        log("ofxWatchdog [parent] detects hangup.");
                    }
                    break;
            }
            if (signal) {
                kill(g_pid, SIGKILL);
                waitpid(g_pid, NULL, 0);
                *code = EXIT_FAILURE;
                result = true;
            }
            else {
                usleep(1000);
                continue;
            }
        }
        else if (pid < 0) {
            error("ofxWatchdog [parent] strangely already no child.");
        }
        break;
    }
    return result;
}

bool ofxWatchdog::child(void)
{
    bool result(false);
    
    if (install()) {
        ofAddListener(ofEvents().setup, &_singleton, &ofxWatchdog::onSetup);
        ofAddListener(ofEvents().update, &_singleton, &ofxWatchdog::onUpdate);
        result = true;
    }
    return result;
}

void ofxWatchdog::onSetup(ofEventArgs& event)
{
    if (!install()) {
        error("ofxWatchdog [child] overwriting signal handler failed.");
    }
    return;
}

void ofxWatchdog::onUpdate(ofEventArgs& event)
{
    clear();
    return;
}

bool ofxWatchdog::install(void)
{
    bool result(false);
    
    if (g_override) {
        if (sigAction(SIGILL, &onSigILL, SA_RESTART | SA_ONSTACK)) {
            if (sigAction(SIGABRT, &onSigABRT, SA_RESTART | SA_ONSTACK)) {
                if (sigAction(SIGFPE, &onSigFPE, SA_RESTART | SA_ONSTACK)) {
                    if (sigAction(SIGBUS, &onSigBUS, SA_RESTART | SA_ONSTACK)) {
                        if (sigAction(SIGSEGV, &onSigSEGV, SA_RESTART | SA_ONSTACK)) {
                            result = true;
                        }
                        else {
                            error("ofxWatchdog [child] setting SIGSEGV handler failed.");
                        }
                    }
                    else {
                        error("ofxWatchdog [child] setting SIGBUS handler failed.");
                    }
                }
                else {
                    error("ofxWatchdog [child] setting SIGFPE handler failed.");
                }
            }
            else {
                error("ofxWatchdog [child] setting SIGABRT handler failed.");
            }
        }
        else {
            error("ofxWatchdog [child] setting SIGILL handler failed.");
        }
    }
    else {
        result = true;
    }
    return result;
}

bool ofxWatchdog::sigAction(int signal, void(*handler)(int, siginfo_t*, void*), int flag)
{
    struct sigaction action;
    bool result(false);
    
    action.sa_sigaction = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = flag | SA_SIGINFO;
    if (sigaction(signal, &action, NULL) == 0) {
        result = true;
    }
    return result;
}

void ofxWatchdog::onSigILL(int signal, siginfo_t* info, void* context)
{
    static char const beacon = 'I';
    
    write(g_pfd, &beacon, sizeof(beacon));
    pause();
    return;
}

void ofxWatchdog::onSigABRT(int signal, siginfo_t* info, void* context)
{
    static char const beacon = 'A';
    
    write(g_pfd, &beacon, sizeof(beacon));
    pause();
    return;
}

void ofxWatchdog::onSigFPE(int signal, siginfo_t* info, void* context)
{
    static char const beacon = 'F';
    
    write(g_pfd, &beacon, sizeof(beacon));
    pause();
    return;
}

void ofxWatchdog::onSigBUS(int signal, siginfo_t* info, void* context)
{
    static char const beacon = 'B';
    
    write(g_pfd, &beacon, sizeof(beacon));
    pause();
    return;
}

void ofxWatchdog::onSigSEGV(int signal, siginfo_t* info, void* context)
{
    static char const beacon = 'S';
    
    write(g_pfd, &beacon, sizeof(beacon));
    pause();
    return;
}

void ofxWatchdog::log(char const* message)
{
    if (g_verbose) {
        std::cout << message << std::endl;
    }
    return;
}

void ofxWatchdog::error(char const* message)
{
    if (g_verbose) {
        std::cerr << message << std::endl;
    }
    exit(EXIT_FAILURE);
    return;
}
