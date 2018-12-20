/*
**      ofxWatchdog version 2
**
**      Original Copyright (C) 2013 - 2018 HORIGUCHI Junshi.
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
**      Xcode 10.1 (Apple LLVM 10.0.0)
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
#include <typeinfo>
#include <fcntl.h>
#include <sys/wait.h>
#ifdef TARGET_OSX
#include <crt_externs.h>
#include <mach-o/dyld.h>
#endif
#ifdef TARGET_LINUX
#include <linux/limits.h>
#endif

ofxWatchdog ofxWatchdog::_singleton;
//  initial -> pid == -1
//   parent -> pid != ::getpid()
//    child -> pid == 0
// daughter -> pid == ::getpid()
static sig_atomic_t volatile g_pid;
//  initial -> pfd == -1
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

void ofxWatchdog::boot(int msec, bool reboot, bool override, bool verbose)
{
    char const* env;
    bool mask;
    int pfd[2];
#if defined TARGET_OSX
    ProcessSerialNumber psn;
#endif
    int code;
    int temp[2];

    g_override = override;
    g_verbose = verbose;
    if ((env = ::getenv(typeid(_singleton).name())) == NULL) {
        if (sigMask(SIGCHLD, true, &mask)) {
            while (true) {
                if (::pipe(pfd) == 0) {
                    g_pid = ::fork();
                    if (g_pid > 0) {
                        ::close(pfd[1]);
                        g_pfd = pfd[0];
                        if (::fcntl(g_pfd, F_SETFL, O_NONBLOCK) == 0) {
#if defined TARGET_OSX
                            ::GetCurrentProcess(&psn);
                            ::TransformProcessType(&psn, kProcessTransformToBackgroundApplication);
#endif
                            if (parent(msec, &code) && reboot) {
                                terminate();
                                log("ofxWatchdog [parent] reboots the child process.");
                                initialize();
#if defined TARGET_OSX
                                ::TransformProcessType(&psn, kProcessTransformToForegroundApplication);
#endif
                                ::usleep(1000000);
                                continue;
                            }
                            else {
                                terminate();
                                log("ofxWatchdog [parent] shuts down the child process.");
                                ::exit(code);
                            }
                        }
                        else {
                            error("ofxWatchdog [parent] controling pipe failed.");
                        }
                    }
                    else if (g_pid == 0) {
                        ::close(pfd[0]);
                        g_pfd = pfd[1];
                        if (::fcntl(g_pfd, F_SETFL, O_NONBLOCK) == 0) {
                            if (sigMask(SIGCHLD, mask, NULL)) {
                                child();
                            }
                            else {
                                error("ofxWatchdog [child] setting signal mask failed.");
                            }
                        }
                        else {
                            error("ofxWatchdog [child] controling pipe failed.");
                        }
                    }
                    else {
                        ::close(pfd[0]);
                        ::close(pfd[1]);
                        error("ofxWatchdog [parent] forking child failed.");
                    }
                }
                else {
                    error("ofxWatchdog [parent] allocating pipe failed.");
                }
                break;
            }
        }
        else {
            error("ofxWatchdog [parent] setting signal mask failed.");
        }
    }
    else if (::sscanf(env, "%d:%d:%d:%d", &pfd[0], &pfd[1], &temp[0], &temp[1]) == 4) {
        if (pfd[0] == ::getpid() && ::dup2(pfd[1], pfd[1]) >= 0 && temp[0] == override && temp[1] == verbose) {
            g_pid = pfd[0];
            g_pfd = pfd[1];
            daughter();
        }
        else {
            error("ofxWatchdog [daughter] checking environment variable failed.");
        }
    }
    else {
        error("ofxWatchdog [daughter] getting environment variable failed.");
    }
    return;
}

void ofxWatchdog::trap(void)
{
    ofAddListener(ofEvents().setup, &_singleton, &ofxWatchdog::onSetup);
    ofAddListener(ofEvents().update, &_singleton, &ofxWatchdog::onUpdate);
    return;
}

void ofxWatchdog::clear(void)
{
    static char const s_beacon = '.';

    if (g_pid == ::getpid()) {
        ::write(g_pfd, &s_beacon, sizeof(s_beacon));
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
        ::close(g_pfd);
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
        if ((pid = ::waitpid(g_pid, &status, WNOHANG)) > 0) {
            if (WIFEXITED(status)) {
                if ((*code = WEXITSTATUS(status)) == EXIT_SUCCESS) {
                    log("ofxWatchdog [parent] externally detects exit.");
                }
                else {
                    ::snprintf(temp, sizeof(temp), "ofxWatchdog [parent] externally detects error exit : %d.", *code);
                    log(temp);
                    result = true;
                }
            }
            else if (WIFSIGNALED(status)) {
                *code = EXIT_FAILURE;
                result = true;
                switch (WTERMSIG(status)) {
                    case SIGTRAP:
                        log("ofxWatchdog [parent] externally detects SIGTRAP, process shutdown.");
                        result = false;
                        break;
                    case SIGILL:
                        log("ofxWatchdog [parent] externally detects SIGILL.");
                        break;
                    case SIGABRT:
                        log("ofxWatchdog [parent] externally detects SIGABRT.");
                        break;
                    case SIGFPE:
                        log("ofxWatchdog [parent] externally detects SIGFPE.");
                        break;
                    case SIGBUS:
                        log("ofxWatchdog [parent] externally detects SIGBUS.");
                        break;
                    case SIGSEGV:
                        log("ofxWatchdog [parent] externally detects SIGSEGV.");
                        break;
                    default:
                        ::snprintf(temp, sizeof(temp), "ofxWatchdog [parent] externally detects unknown signal : %d.", WTERMSIG(status));
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
            if (::read(g_pfd, &beacon, sizeof(beacon)) <= 0) {
                beacon = 'X';
            }
            switch (beacon) {
                case '.':
                    hangup = 0;
                    signal = false;
                    break;
                case 'I':
                    log("ofxWatchdog [parent] internally detects SIGILL.");
                    break;
                case 'A':
                    log("ofxWatchdog [parent] internally detects SIGABRT.");
                    break;
                case 'F':
                    log("ofxWatchdog [parent] internally detects SIGFPE.");
                    break;
                case 'B':
                    log("ofxWatchdog [parent] internally detects SIGBUS.");
                    break;
                case 'S':
                    log("ofxWatchdog [parent] internally detects SIGSEGV.");
                    break;
                default:
                    if (++hangup < msec) {
                        signal = false;
                    }
                    else {
                        log("ofxWatchdog [parent] externally detects hangup.");
                    }
                    break;
            }
            if (signal) {
                ::kill(g_pid, SIGKILL);
                if (::waitpid(g_pid, NULL, 0) > 0) {
                    *code = EXIT_FAILURE;
                    result = true;
                }
                else {
                    signal = false;
                }
            }
            if (!signal) {
                ::usleep(1000);
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

void ofxWatchdog::child(void)
{
    char temp[256];
    char path[PATH_MAX];
    uint32_t size;
    char const* const* argv;
#if defined TARGET_OSX
    char*** argp;
#elif defined TARGET_LINUX
    FILE* fp;
    char* ap;
    size_t as;
    vector<string> args;
    vector<char const*> argp;
#endif

    ::snprintf(temp, sizeof(temp), "%d:%d:%d:%d", ::getpid(), g_pfd, g_override, g_verbose);
    if (::setenv(typeid(_singleton).name(), temp, true) == 0) {
        size = 0;
        argv = NULL;
#if defined TARGET_OSX
        size = sizeof(path);
        if (::_NSGetExecutablePath(path, &size) == 0) {
            if ((argp = ::_NSGetArgv()) != NULL) {
                argv = *argp;
            }
            else {
                error("ofxWatchdog [child] getting argv failed.");
            }
        }
        else {
            error("ofxWatchdog [child] getting path failed.");
        }
#elif defined TARGET_LINUX
        ::snprintf(temp, sizeof(temp), "/proc/%d/exe", ::getpid());
        if ((size = ::readlink(temp, path, sizeof(path) - 1)) >= 0) {
            path[size] = '\0';
            ::snprintf(temp, sizeof(temp), "/proc/%d/cmdline", ::getpid());
            if ((fp = ::fopen(temp, "rb")) != NULL) {
                ap = NULL;
                as = 0;
                while (::getdelim(&ap, &as, '\0', fp) >= 0) {
                    args.push_back(ap);
                    argp.push_back(args.back().c_str());
                }
                ::free(ap);
                argp.push_back(NULL);
                argv = argp.data();
                ::fclose(fp);
            }
            else {
                error("ofxWatchdog [child] getting argv failed.");
            }
        }
        else {
            error("ofxWatchdog [child] getting path failed.");
        }
#else
#error "algorithm not implemented"
#endif
        if (size > 0 && argv != NULL) {
            if (::execv(path, const_cast<char* const*>(argv)) == 0) {
                error("ofxWatchdog [child] fatal condition error.");
            }
            else {
                error("ofxWatchdog [child] executing daughter failed.");
            }
        }
        else {
            error("ofxWatchdog [child] fatal condition error.");
        }
    }
    else {
        error("ofxWatchdog [child] setting environment variable failed.");
    }
    return;
}

void ofxWatchdog::daughter(void)
{
    static char s_stack[SIGSTKSZ];
    stack_t stack;

    if (g_override) {
        stack.ss_sp = &s_stack;
        stack.ss_size = sizeof(s_stack);
        stack.ss_flags = 0;
        if (::sigaltstack(&stack, NULL) == 0) {
            if (sigAction(SIGILL, &onSigILL, SA_RESTART | SA_ONSTACK)) {
                if (sigAction(SIGABRT, &onSigABRT, SA_RESTART | SA_ONSTACK)) {
                    if (sigAction(SIGFPE, &onSigFPE, SA_RESTART | SA_ONSTACK)) {
                        if (sigAction(SIGBUS, &onSigBUS, SA_RESTART | SA_ONSTACK)) {
                            if (!sigAction(SIGSEGV, &onSigSEGV, SA_RESTART | SA_ONSTACK)) {
                                error("ofxWatchdog [daughter] setting SIGSEGV handler failed.");
                            }
                        }
                        else {
                            error("ofxWatchdog [daughter] setting SIGBUS handler failed.");
                        }
                    }
                    else {
                        error("ofxWatchdog [daughter] setting SIGFPE handler failed.");
                    }
                }
                else {
                    error("ofxWatchdog [daughter] setting SIGABRT handler failed.");
                }
            }
            else {
                error("ofxWatchdog [daughter] setting SIGILL handler failed.");
            }
        }
        else {
            error("ofxWatchdog [daughter] allocating signal stack failed.");
        }
    }
    return;
}

void ofxWatchdog::onSetup(ofEventArgs& event)
{
    daughter();
    return;
}

void ofxWatchdog::onUpdate(ofEventArgs& event)
{
    clear();
    return;
}

bool ofxWatchdog::sigMask(int signal, bool set, bool* get)
{
    sigset_t mask;
    sigset_t save;
    bool result(false);

    sigemptyset(&mask);
    sigaddset(&mask, signal);
    if (::sigprocmask((set) ? (SIG_BLOCK) : (SIG_UNBLOCK), &mask, &save) == 0) {
        if (get != NULL) {
            *get = sigismember(&save, signal);
        }
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
    if (::sigaction(signal, &action, NULL) == 0) {
        result = true;
    }
    return result;
}

void ofxWatchdog::onSigILL(int signal, siginfo_t* info, void* context)
{
    static char const s_beacon = 'I';

    ::write(g_pfd, &s_beacon, sizeof(s_beacon));
    terminate();
    while (true) {
        ::pause();
    }
    return;
}

void ofxWatchdog::onSigABRT(int signal, siginfo_t* info, void* context)
{
    static char const s_beacon = 'A';

    ::write(g_pfd, &s_beacon, sizeof(s_beacon));
    terminate();
    while (true) {
        ::pause();
    }
    return;
}

void ofxWatchdog::onSigFPE(int signal, siginfo_t* info, void* context)
{
    static char const s_beacon = 'F';

    ::write(g_pfd, &s_beacon, sizeof(s_beacon));
    terminate();
    while (true) {
        ::pause();
    }
    return;
}

void ofxWatchdog::onSigBUS(int signal, siginfo_t* info, void* context)
{
    static char const s_beacon = 'B';

    ::write(g_pfd, &s_beacon, sizeof(s_beacon));
    terminate();
    while (true) {
        ::pause();
    }
    return;
}

void ofxWatchdog::onSigSEGV(int signal, siginfo_t* info, void* context)
{
    static char const s_beacon = 'S';

    ::write(g_pfd, &s_beacon, sizeof(s_beacon));
    terminate();
    while (true) {
        ::pause();
    }
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
    terminate();
    if (g_pid == 0) {
        ::_exit(EXIT_FAILURE);
    }
    else {
        ::exit(EXIT_FAILURE);
    }
    return;
}
