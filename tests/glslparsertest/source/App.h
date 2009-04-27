//
// Author:    Philip Rideout
// Copyright: 2002-2005  3Dlabs Inc. Ltd.  All rights reserved.
// License:   see 3Dlabs-license.txt
//

#ifndef APP_H
#define APP_H
#include <GL/glew.h>
#include <wx/wx.h>
#include <wx/cmdline.h>
#include <stdarg.h>
#include "os.h"
class TFrame;

enum TResult {Unassigned, NonGL2, Success, Error, Crash};

// Application class.  Globally accessible via wxGetApp().
class TApp : public wxApp {
  public:
    bool OnInit();
    int OnExit();
    void SetFrame(TFrame* frame) { this->frame = frame; }
    void Errorf(const char* format, ...);
    wxString LogFile() const { return logfile; }
    void SetExitCode(TResult code) { this->code = code; }
    bool IsQuick() const { return quick; }
    bool IsServer() const { return server; }
    bool IsCrashTolerant() const { return true; }
    static const wxCmdLineEntryDesc CommandLineDescription[];
  private:
    TResult code;
    bool quick;
    bool server;
    wxString logfile;
    TFrame* frame;
};

DECLARE_APP(TApp)

#endif
