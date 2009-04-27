//
// Author:    Philip Rideout
// Copyright: 2002-2005  3Dlabs Inc. Ltd.  All rights reserved.
// License:   see 3Dlabs-license.txt
//

#include <wx/wx.h>
#include <wx/ipc.h>
class TCanvas;
class TServer;

#define IPC_SERVICE "4242"
#define IPC_TOPIC "GLSL Parser Test"

static char pReady[] = "Ready";
static char pSuccess[] = "Success";
static char pFailure[] = "Failure";
static char pGoodbye[] = "Goodbye";
static char pCrash[] = "Crash";

class TServerConnection : public wxConnection {
  public:
    TServerConnection(TCanvas* canvas) :  dead(false), canvas(canvas) {}
    char* OnRequest(const wxString& topic, const wxString& item, int* size, wxIPCFormat format);
    bool Dead() const { return dead; }
  private:
    bool dead;
    TCanvas* canvas;
};

class TClientConnection : public wxConnection {};

class TClient : public wxClient {
  public:
    wxConnectionBase* OnMakeConnection() { return new TClientConnection; }
};

class TServer : public wxServer {
  public:
    TServer(TCanvas* canvas) : connection(0), dead(false), canvas(canvas) {}
    wxConnectionBase* OnAcceptConnection(const wxString& topic);
    bool Dead() const { return connection && connection->Dead(); }
  private:
    TServerConnection* connection;
    TCanvas* canvas;
    bool dead;
};
