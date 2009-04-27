//
// Author:    Philip Rideout
// Copyright: 2002-2005  3Dlabs Inc. Ltd.  All rights reserved.
// License:   see 3Dlabs-license.txt
//

#include "App.h"
#include "Canvas.h"
#include "Frame.h"
#include "Tables.h"
#include "Ipc.h"
#include <wx/ffile.h>
#include <fcntl.h>
#include <io.h>
#include <math.h>
#include <wx/process.h>
#include <wx/utils.h>
#include <GL/glew.h>

#define DELAY 500

BEGIN_EVENT_TABLE(TCanvas, wxGLCanvas)
    EVT_SIZE(TCanvas::OnSize)
    EVT_PAINT(TCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(TCanvas::OnEraseBackground)
END_EVENT_TABLE()

int TCanvas::Attributes[] = 
{
    WX_GL_RGBA,
    WX_GL_DOUBLEBUFFER,
    0
};

TCanvas::TCanvas(wxWindow *parent) : wxGLCanvas(parent, (wxGLCanvas*) 0, Id::WidgetCanvas, wxDefaultPosition, wxDefaultSize, 0, "TCanvas", Attributes)
{
    glReady = false;
    frame = (TFrame*) parent;
    passed = failed = 0;
    wxStartTimer();

    if (wxGetApp().IsServer())
        glSetup();
}

void TCanvas::OnSize(wxSizeEvent& event)
{
    wxGLCanvas::OnSize(event);

    if (GetContext() && glReady)
        glSetup();
}

void TCanvas::glSetup()
{
    GetClientSize(&width, &height);
    left = -1.1f;
    right = 1.1f;
    bottom = -2.5f;
    top = 2.5f;
    znear = 2;
    zfar = 10;

    SetCurrent();
    glClearColor(1, 1, .8f, 1);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(left, right, bottom, top, znear, zfar);
    glMatrixMode(GL_MODELVIEW);

    if (glReady)
        return;

    // Initialize the "OpenGL Extension Wrangler" library
    glewInit();

    // Abort if OpenGL support is less than 2.0
	int major, minor;
    const GLubyte* string = glGetString(GL_VERSION);
    sscanf(reinterpret_cast<const char*>(string), "%d.%d", &major, &minor);
    if (major <= 1)
	{
        wxGetApp().SetExitCode(NonGL2);
        frame->Close();
        return;
    }

	// Continue with OpenGL initialization
    InitSuite();

    vertex = glCreateShader(GL_VERTEX_SHADER);
    fragment = glCreateShader(GL_FRAGMENT_SHADER);

    if (wxGetApp().IsServer()) {
        TServer server(this);
        server.Create(IPC_SERVICE);
        while (!server.Dead())
            wxSafeYield();

        frame->Close();
        return;
    }

    Header();

    RunTests();

    Footer();

    if (wxGetApp().IsQuick()) {
        frame->Save("../" + wxGetApp().LogFile());
        frame->Close();
    }

    glReady = true;
}

void TCanvas::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    if (!GetContext())
        return;
    SetCurrent();
    if (!glReady)
        glSetup();

    glClear(GL_COLOR_BUFFER_BIT);
    Draw();
    SwapBuffers();
}

void TCanvas::Draw() const
{
    glPushMatrix();
    glTranslatef(0,0,-5);

    float x = -1;
    float dx = 2.0f / (suite.size() - 1);
    glBegin(GL_QUAD_STRIP);
    for (unsigned int i = 0; i < suite.size(); ++i) {

        if (suite[i].actual == Unassigned)
            glColor3f(0, 0, 0);
        else if (suite[i].actual == Crash || suite[i].actual != suite[i].expected)
            glColor3f(1, 0, 0);
        else {
            float center = suite.size() / 2.0f;
            float blue = fabsf(i - center) / center;
            glColor3f(0, blue, 1 - blue);
        }

        glVertex2f(x, -1);
        glVertex2f(x, 1);
        x += dx;
    }
    glEnd();

    glColor3f(0,0,0);
    glBegin(GL_LINE_STRIP);
    glVertex2f(-1, -1);
    glVertex2f(-1, 1);
    glVertex2f(1, 1);
    glVertex2f(1, -1);
    glVertex2f(-1, -1);
    glEnd();

    glPopMatrix();
}

void TCanvas::Footer() const
{
    frame->Flush(EBody);

    frame->Printf(EFooter, "<br><table cellspacing=0 cellpadding=0 border=0>\n");
    frame->Printf(EFooter, "<tr><td width=10><font size=-1 color=#00a000><b>pass&nbsp </b></font><td><font size=-1>The GLSL implementation parsed this shader correctly.</font>\n");
    frame->Printf(EFooter, "<tr><td width=10><font size=-1 color=#a00000><b>fail&nbsp </b></font><td><font size=-1>The GLSL implementation did not parse this shader correctly.</font>\n");
    frame->Printf(EFooter, "<tr><td width=10><font size=-1 color=#ff0000><b>crash&nbsp </b></font><td><font size=-1>The GLSL implementation crashed while parsing this shader.</font>\n");
    frame->Printf(EFooter, "</table><p>\n");

    frame->Printf(EFooter, "<b>passed:</b> %d<br>\n", passed);
    frame->Printf(EFooter, "<b>failed:</b> %d<br>\n", failed);
    frame->Printf(EFooter, "<b>score:</b>  %3.0f%%<br>\n", 100 * (float) passed / (passed + failed));

    wxDateTime now = wxDateTime::Now();
    frame->Printf(EFooter, "<address><font color=#0000a0 size=-2>%s %s</font></address>", now.FormatDate(), now.FormatTime());
    frame->Flush(EFooter);
}

void TCanvas::InitSuite()
{
    TTest test;
    int expected;
    char buffer[128];
    wxFFile infile("../suite.txt");
    test.actual = Unassigned;

    fgets(buffer, sizeof(buffer) - 1, infile.fp());
    while (buffer[0] == ' ' || buffer[0] == '\n'  || buffer[0] == ';')
        fgets(buffer, sizeof(buffer) - 1, infile.fp());

    if (buffer[strlen(buffer) - 1] == 10)
        buffer[strlen(buffer) - 1] = 0;

    folder = wxString(buffer);

    while (!infile.Eof()) {
        if (2 != fscanf(infile.fp(), "%d %s", &expected, buffer))
            break;

        test.expected = expected ? Success : Error;
        test.shader = wxString(buffer);
        suite.push_back(test);
    }
}

void TCanvas::Header() const
{
    frame->Printf(EHeader, "<h3>GLSL Parser Test</h3>\n");
    frame->Printf(EHeader, "<b>vendor: </b>%s<br>", glGetString(GL_VENDOR));
    frame->Printf(EHeader, "<b>renderer: </b>%s<br>", glGetString(GL_RENDERER));
    frame->Printf(EHeader, "<b>OpenGL version: </b>%s\n", glGetString(GL_VERSION));
    frame->Flush(EHeader);

    frame->Printf(EBody, "<table cellspacing=0 cellpadding=0 border=0>\n");
    frame->Printf(EBody, "<tr>\n");
    frame->Printf(EBody, "<th align=left width=200><b><font size=-1>shader</font></b></th>\n");
    frame->Printf(EBody, "<th align=left><b><font size=-1>&nbsp expected</font></b></th>\n");
    frame->Printf(EBody, "<th align=left><b><font size=-1>&nbsp actual</font></b></th>\n");
    frame->Printf(EBody, "<th align=left><b><font size=-1>&nbsp</font></b></th>\n");
    frame->Printf(EBody, "</tr>\n");
}

wxProcess* TCanvas::Connect(TClient*& client, TClientConnection*& connection)
{
    wxLogNull suppress;
    wxProcess* server;

    // Start the server process.
    server = wxProcess::Open("glslparsertest /s");
    client = new TClient;

    // Try to establish a connection.
    while (!connection) {
        wxSafeYield();
        connection = (TClientConnection*) client->MakeConnection("localhost", IPC_SERVICE, IPC_TOPIC);
    }
    return server;
}

void TCanvas::RunTests()
{
    TClient* client = 0;
    TClientConnection* connection = 0;
    wxProcess* server = 0;

    if (wxGetApp().IsCrashTolerant())
        server = Connect(client, connection);

    for (TSuite::iterator i = suite.begin(); i != suite.end(); ++i) {

        bool crash = false;
        wxString fullpath = wxString("..") + wxFileName::GetPathSeparator() + folder + wxFileName::GetPathSeparator() + i->shader;

        if (connection) {
            char* response = connection->Request(fullpath);

            if (!strcmp(response, pCrash)) {

                connection = 0;

                // Wait half a second to allow it to terminate.
                wxGetElapsedTime(true);
                while (wxGetElapsedTime(false) < DELAY)
                    wxSafeYield();

                delete client;
                server = Connect(client, connection);
                crash = true;
            }

            success = !strcmp(response, pSuccess);
        }
        else {
            try {
                Compile(fullpath);
            }
            catch (...) {
                crash = true;
                success = false;
            }
        }

        i->actual = success ? Success : Error;
        if (crash)
            i->actual = Crash;

        frame->Printf(EBody, "<tr>\n");

        // Shader
        wxFileName absolute = fullpath;
        absolute.MakeAbsolute();
        frame->Printf(EBody, "<td width=200><font size=-1><a href='%s'>%s</a></font></td>\n", absolute.GetFullPath(), i->shader);

        // Expected result
        frame->Printf(EBody, "<td><font size=-1>&nbsp ");
        if (i->expected == Success)
            frame->Printf(EBody, "success");
        else
            frame->Printf(EBody, "error");
        frame->Printf(EBody, "</font></td>\n");

        // Actual result
        frame->Printf(EBody, "<td><font size=-1>&nbsp ");
        if (success)
            frame->Printf(EBody, "success");
        else if (crash)
            frame->Printf(EBody, "<font color=#ff0000><b>crash</b></font>");
        else
            frame->Printf(EBody, "error");
        frame->Printf(EBody, "</font></td>\n");

        // Pass or fail
        frame->Printf(EBody, "<td><font size=-1 color=");
        if (i->actual == i->expected && !crash) {
            ++passed;
            frame->Printf(EBody, "#00a000><b>&nbsp pass");
        } else {
            ++failed;
            frame->Printf(EBody, "#a00000><b>&nbsp fail");
        }
        frame->Printf(EBody, "</b></font></td>\n");

        frame->Printf(EBody, "</tr>\n");

        glClear(GL_COLOR_BUFFER_BIT);
        Draw();
        SwapBuffers();
    }

    if (connection) {
        connection->Request("die");
        delete client;
    }
}

void TCanvas::ShowDialog(const char* filename, const char* infolog, const char* source)
{
    wxString title = "InfoLog for '" + wxString(filename) + "'";
    wxMessageBox(infolog, title);
}

bool TCanvas::Compile(const wxString& filename, bool show)
{
    GLuint shader = (filename[filename.length() - 1] == 't') ? vertex : fragment;

    int file, size;
    char* source;
    const char* pFilename = filename.c_str();

    file = _open(pFilename, _O_RDONLY);
    if (file == -1) {
        wxGetApp().Errorf("Unable to open file.\n");
        return success = false;
    }
    size = _lseek(file, 0, SEEK_END);
    source = (GLchar*) malloc(size + 1);
    _lseek(file, 0, SEEK_SET);
    size = _read(file, source, size);
    _close(file);
    source[size] = 0;
    glShaderSource(shader, 1, (const GLchar**) &source, 0);

    int code;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &code);

    if (show) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* infolog = new char[length];
        glGetShaderInfoLog(shader, length, 0, infolog);

        if (!infolog || !*infolog) {
            delete infolog;
            infolog = new char[8];
            strcpy(infolog, "<empty>");
        }

        ShowDialog(filename, infolog, source);
        delete infolog;
    }

    free(source);
    return success = (code ? true : false);
}

const TTest* TCanvas::Find(wxString shader) const
{
    wxFileName request = shader;
    for (TSuite::const_iterator i = suite.begin(); i != suite.end(); ++i) {
        wxFileName target = i->shader;
        if (target.GetFullName() == request.GetFullName())
            return &*i;
    }
    return 0;
}
