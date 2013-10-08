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
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>

ofxWatchdog ofxWatchdog::_singleton;
static char g_stack[SIGSTKSZ];
static sig_atomic_t volatile g_pid;
static sig_atomic_t volatile g_pfd;
static sig_atomic_t volatile g_live;
static sig_atomic_t volatile g_kill;

ofxWatchdog::ofxWatchdog(void)
{
    g_pid = -1;
}

ofxWatchdog::~ofxWatchdog(void)
{
    if (g_pid >= 0) {
        close(g_pfd);
    }
}

bool ofxWatchdog::watch(int msec, bool reboot, bool verbose)
{
    stack_t stack;
    int pfd[2];
    bool result(false);
    
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
                        if (parent(msec, verbose) && reboot) {
                            close(g_pfd);
                            g_pid = -1;
                            continue;
                        }
                    }
                    else {
                        error(verbose, "ofxWatchdog [parent] controling pipe failed.");
                    }
                }
                else if (g_pid == 0) {
                    close(pfd[0]);
                    g_pfd = pfd[1];
                    if (fcntl(g_pfd, F_SETFL, O_NONBLOCK) == 0) {
                        if (child(verbose)) {
                            usleep(100000);
                            result = true;
                        }
                    }
                    else {
                        error(verbose, "ofxWatchdog [child] controling pipe failed.");
                    }
                }
                else {
                    close(pfd[0]);
                    close(pfd[1]);
                    error(verbose, "ofxWatchdog [parent] forking failed.");
                }
            }
            else {
                error(verbose, "ofxWatchdog [parent] allocating pipe failed.");
            }
            break;
        }
    }
    else {
        error(verbose, "ofxWatchdog [parent] allocating stack failed.");
    }
    return result;
}

void ofxWatchdog::clear(void)
{
    static char const state = '.';
    
    if (g_pid == 0) {
        write(g_pfd, &state, sizeof(state));
    }
    return;
}

bool ofxWatchdog::parent(int msec, bool verbose)
{
    int hangup;
    bool signal;
    char state;
    bool result(false);
    
    g_live = true;
    g_kill = false;
    if (sigAction(SIGCHLD, &onSigCHLD, SA_NOCLDSTOP | SA_RESTART | SA_ONSTACK)) {
        hangup = 0;
        while (g_live) {
            signal = true;
            if (read(g_pfd, &state, sizeof(state)) <= 0) {
                state = 'X';
            }
            switch (state) {
                case '.':
                    hangup = 0;
                    signal = false;
                    break;
                case 'I':
                    log(verbose, "ofxWatchdog [parent] detects SIGILL.");
                    break;
                case 'F':
                    log(verbose, "ofxWatchdog [parent] detects SIGFPE.");
                    break;
                case 'B':
                    log(verbose, "ofxWatchdog [parent] detects SIGBUS.");
                    break;
                case 'S':
                    log(verbose, "ofxWatchdog [parent] detects SIGSEGV.");
                    break;
                default:
                    if (++hangup < msec) {
                        signal = false;
                    }
                    else {
                        log(verbose, "ofxWatchdog [parent] detects hangup.");
                    }
                    break;
            }
            if (signal) {
                g_kill = true;
                kill(g_pid, SIGTERM);
                waitpid(g_pid, NULL, 0);
                break;
            }
            usleep(1000);
        }
        if (g_live) {
            result = true;
        }
    }
    else {
        error(verbose, "ofxWatchdog [parent] setting SIGCHLD failed.");
    }
    return result;
}

bool ofxWatchdog::child(bool verbose)
{
    bool result(false);
    
    if (sigAction(SIGILL, &onSigILL, SA_RESTART | SA_ONSTACK)) {
        if (sigAction(SIGFPE, &onSigFPE, SA_RESTART | SA_ONSTACK)) {
            if (sigAction(SIGBUS, &onSigBUS, SA_RESTART | SA_ONSTACK)) {
                if (sigAction(SIGSEGV, &onSigSEGV, SA_RESTART | SA_ONSTACK)) {
                    result = true;
                }
                else {
                    error(verbose, "ofxWatchdog [child] setting SIGSEGV failed.");
                }
            }
            else {
                error(verbose, "ofxWatchdog [child] setting SIGBUS failed.");
            }
        }
        else {
            error(verbose, "ofxWatchdog [child] setting SIGFPE failed.");
        }
    }
    else {
        error(verbose, "ofxWatchdog [child] setting SIGILL failed.");
    }
    return result;
}

void ofxWatchdog::log(bool verbose, char const* message)
{
    if (verbose) {
        std::cout << message << std::endl;
    }
    return;
}

void ofxWatchdog::error(bool verbose, char const* message)
{
    if (verbose) {
        std::cerr << message << std::endl;
    }
    if (g_pid == 0) {
        _exit(EXIT_FAILURE);
    }
    else {
        exit(EXIT_FAILURE);
    }
    return;
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

void ofxWatchdog::onSigCHLD(int signal, siginfo_t* info, void* param)
{
    if (info->si_code == CLD_EXITED) {
        if (!g_kill) {
            g_live = false;
        }
        g_kill = false;
    }
    return;
}

void ofxWatchdog::onSigILL(int signal, siginfo_t* info, void* param)
{
    static char const state = 'I';
    
    write(g_pfd, &state, sizeof(state));
    pause();
    return;
}

void ofxWatchdog::onSigFPE(int signal, siginfo_t* info, void* param)
{
    static char const state = 'F';
    
    write(g_pfd, &state, sizeof(state));
    pause();
    return;
}

void ofxWatchdog::onSigBUS(int signal, siginfo_t* info, void* param)
{
    static char const state = 'B';
    
    write(g_pfd, &state, sizeof(state));
    pause();
    return;
}

void ofxWatchdog::onSigSEGV(int signal, siginfo_t* info, void* param)
{
    static char const state = 'S';
    
    write(g_pfd, &state, sizeof(state));
    pause();
    return;
}
