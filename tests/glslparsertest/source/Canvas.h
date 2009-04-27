//
// Author:    Philip Rideout
// Copyright: 2002-2005  3Dlabs Inc. Ltd.  All rights reserved.
// License:   see 3Dlabs-license.txt
//

#ifndef CANVAS_H
#define CANVAS_H
#include "App.h"
#include <wx/glcanvas.h>
#include <vector>
class TFrame;
class TServer;
class TClient;
class TClientConnection;

struct TTest {
    wxString shader;
    TResult actual;
    TResult expected;
};

typedef std::vector<TTest> TSuite;

// GUI widget containing the GL viewport.
class TCanvas : public wxGLCanvas {
  public:
    TCanvas(wxWindow *parent);
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event) {}
    void OnSize(wxSizeEvent& event);
    static wxProcess* Connect(TClient*& client, TClientConnection*& connection);
    void RunTests();
    void InitSuite();
    void Draw() const;
    void Header() const;
    void Footer() const;
    bool Compile(const wxString& filename, bool show = false);
    const TTest* Find(wxString shader) const;
    static void ShowDialog(const char* filename, const char* infolog, const char* source);
    static int Attributes[];
  private:
    void glSetup();
    bool glReady;
    bool success;
    int width, height;
    int passed, failed;
    float left, right, bottom, top, znear, zfar;
    TFrame* frame;
    TServer* server;
    TSuite suite;
    wxString folder;
    GLuint vertex;
    GLuint fragment;
    DECLARE_EVENT_TABLE()
};

#endif
