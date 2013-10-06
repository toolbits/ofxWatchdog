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
static volatile sig_atomic_t g_live;
static volatile sig_atomic_t g_kill;
static volatile sig_atomic_t g_verbose;
static int g_pid;
static int g_pfd[2];

ofxWatchdog::ofxWatchdog(void)
{
    g_live = false;
    g_kill = false;
    g_verbose = false;
    g_pid = -1;
}

ofxWatchdog::~ofxWatchdog(void)
{
    if (g_pid == 0) {
        close(g_pfd[1]);
    }
}

bool ofxWatchdog::watch(int msec, bool reboot, bool verbose)
{
    struct sigaction action;
    int hang;
    char state;
    bool result(false);
    
    g_live = true;
    g_verbose = verbose;
    while (g_live) {
        if (pipe(g_pfd) == 0) {
            g_pid = fork();
            switch (g_pid) {
                case -1:
                    if (g_verbose) {
                        std::cerr << "ofxWatchdog [parent] forking failed." << std::endl;
                    }
                    g_live = false;
                    break;
                case 0:
                    close(g_pfd[0]);
                    fcntl(g_pfd[1], F_SETFL, O_NONBLOCK);
                    
                    action.sa_sigaction = &onSigBUS;
                    sigemptyset(&action.sa_mask);
                    action.sa_flags = SA_RESTART | SA_SIGINFO;
                    if (sigaction(SIGBUS, &action, NULL) == 0) {
                        action.sa_sigaction = &onSigFPE;
                        sigemptyset(&action.sa_mask);
                        action.sa_flags = SA_RESTART | SA_SIGINFO;
                        if (sigaction(SIGFPE, &action, NULL) == 0) {
                            result = true;
                        }
                        else if (g_verbose) {
                            std::cerr << "ofxWatchdog [child] setting SIGFPE failed." << std::endl;
                        }
                    }
                    else if (g_verbose) {
                        std::cerr << "ofxWatchdog [child] setting SIGBUS failed." << std::endl;
                    }
                    g_live = false;
                    break;
                default:
                    close(g_pfd[1]);
                    fcntl(g_pfd[0], F_SETFL, O_NONBLOCK);
                    
                    action.sa_sigaction = &onSigCHLD;
                    sigemptyset(&action.sa_mask);
                    action.sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_SIGINFO;
                    if (sigaction(SIGCHLD, &action, NULL) == 0) {
                        hang = 0;
                        while (g_live) {
                            if (read(g_pfd[0], &state, sizeof(state)) > 0) {
                                hang = 0;
                            }
                            else if (++hang >= msec) {
                                if (g_verbose) {
                                    std::cout << "ofxWatchdog [parent] detects hang up." << std::endl;
                                }
                                g_kill = true;
                                kill(g_pid, SIGTERM);
                                while (g_kill) {
                                    usleep(1000);
                                }
                                break;
                            }
                            usleep(1000);
                        }
                        if (!reboot) {
                            g_live = false;
                        }
                    }
                    else {
                        if (g_verbose) {
                            std::cerr << "ofxWatchdog [parent] setting SIGCHLD failed." << std::endl;
                        }
                        g_live = false;
                    }
                    close(g_pfd[0]);
                    break;
            }
        }
        else {
            if (g_verbose) {
                std::cerr << "ofxWatchdog [parent] allocating pipe failed." << std::endl;
            }
            g_live = false;
        }
    }
    return result;
}

void ofxWatchdog::clear(void)
{
    static char const state = '.';
    
    if (g_pid == 0) {
        write(g_pfd[1], &state, sizeof(state));
    }
    return;
}

void ofxWatchdog::onSigCHLD(int signal, siginfo_t* info, void* param)
{
    if (info->si_pid != 0) {
        if (!g_kill) {
            g_live = false;
        }
        g_kill = false;
    }
    return;
}

void ofxWatchdog::onSigBUS(int signal, siginfo_t* info, void* param)
{
    if (g_verbose) {
        std::cerr << "ofxWatchdog [child] detects SIGBUS." << std::endl;
    }
    while (true) {
        usleep(1000);
    }
    return;
}

void ofxWatchdog::onSigFPE(int signal, siginfo_t* info, void* param)
{
    if (g_verbose) {
        std::cerr << "ofxWatchdog [child] detects SIGFPE." << std::endl;
    }
    while (true) {
        usleep(1000);
    }
    return;
}
