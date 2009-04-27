//
// Author:    Philip Rideout
// Copyright: 2002-2005  3Dlabs Inc. Ltd.  All rights reserved.
// License:   see 3Dlabs-license.txt
//

#include "App.h"
#include "Frame.h"
#include "Canvas.h"
#include <wx/cmdline.h>

const wxCmdLineEntryDesc TApp::CommandLineDescription[] =
{
    { wxCMD_LINE_SWITCH, "q", "quick", "run all tests, save the results, then exit"},
    { wxCMD_LINE_OPTION, "l", "log", "specify the log filename (defaults to 'log.html')"},
    { wxCMD_LINE_SWITCH, "s", "server", "(internal use only)"},
    { wxCMD_LINE_NONE }
};

bool TApp::OnInit()
{
    code = Success;
    wxCmdLineParser parser(argc, argv);
    parser.SetDesc(CommandLineDescription);
    parser.Parse();
    server = parser.Found("s");
    quick = parser.Found("q");
    logfile = "log.html";
    parser.Found("l", &logfile);
    new TFrame("GLSL Parser Test", wxDefaultPosition, wxSize(500,750));
    if (!IsServer())
        frame->Show(true);
    SetTopWindow(frame);
    return true;
} 

void TApp::Errorf(const char* format, ...)
{
    wxString message;

    va_list marker;
    va_start(marker, format);
    message.PrintfV(format, marker);
    frame->Printf(EBody, "<font color=#ff0000>%s</font>\n", message);
    frame->Flush(EBody);
}

int TApp::OnExit()
{
    if (code == NonGL2)
    {
        wxMessageBox(
            "You must have OpenGL 2.0 compliant drivers to run glslparsertest!",
            "OpenGL 2.0 Driver Not Found", wxOK | wxICON_EXCLAMATION, 0
        );
        return 1;
    }

    return (code == Success) ? 0 : 1;
}

IMPLEMENT_APP(TApp)
