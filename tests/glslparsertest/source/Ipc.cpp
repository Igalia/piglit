//
// Author:    Philip Rideout
// Copyright: 2002-2005  3Dlabs Inc. Ltd.  All rights reserved.
// License:   see 3Dlabs-license.txt
//

#include <wx/wx.h>
#include <wx/ipc.h>
#include "Ipc.h"
#include "Canvas.h"

char* TServerConnection::OnRequest(const wxString& topic, const wxString& item, int* size, wxIPCFormat format)
{
    if (item == "die") {
        dead = true;
        return pGoodbye;
    }

    bool success;

    try {
        success = canvas->Compile(item);
    }

    catch (...) {
        dead = true;
        return pCrash;
    }

    return success ? pSuccess : pFailure;
}

wxConnectionBase* TServer::OnAcceptConnection(const wxString& topic)
{
    if (topic == IPC_TOPIC)
        return connection = new TServerConnection(canvas);

    // unknown topic
    return 0;
}
