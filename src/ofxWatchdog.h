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
**      ofxWatchdog.h
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

#ifndef __OFX_WATCHDOG_H
#define __OFX_WATCHDOG_H

#include <signal.h>
#include <ofEvents.h>

class ofxWatchdog {
    private:
        static  ofxWatchdog     _singleton;

    public:
        // Use this function at startup
        //
        //     msec : how long does the watchdog timer wait, when the process hangs-up in milli seconds
        //   reboot : automatically restart the process or not
        // override : use internal signal handler or not (optional)
        //  verbose : print more log information or not (optional)
        static  void            boot            (int msec, bool reboot, bool override = true, bool verbose = true);

        // Use this function after ofSetupOpenGL() function call
        static  void            trap            (void);

        // Use this function continuously before the watchdog timer timeouts (optional)
        // This function is optional, because ofxWatchdog automatically calls this in every update events
        static  void            clear           (void);
    private:
        explicit                ofxWatchdog     (void);
                                ~ofxWatchdog    (void);
        static  void            initialize      (void);
        static  void            terminate       (void);
        static  bool            parent          (int msec, int* code);
        static  void            child           (void);
        static  void            daughter        (void);
                void            onSetup         (ofEventArgs& event);
                void            onUpdate        (ofEventArgs& event);
        static  bool            sigMask         (int signal, bool set, bool* get);
        static  bool            sigAction       (int signal, void(*handler)(int, siginfo_t*, void*), int flag);
        static  void            onSigILL        (int signal, siginfo_t* info, void* context);
        static  void            onSigABRT       (int signal, siginfo_t* info, void* context);
        static  void            onSigFPE        (int signal, siginfo_t* info, void* context);
        static  void            onSigBUS        (int signal, siginfo_t* info, void* context);
        static  void            onSigSEGV       (int signal, siginfo_t* info, void* context);
        static  void            log             (char const* message);
        static  void            error           (char const* message);
    private:
                                ofxWatchdog     (ofxWatchdog const&);
                ofxWatchdog&    operator=       (ofxWatchdog const&);
};

#endif
