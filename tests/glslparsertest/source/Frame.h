//
// Author:    Philip Rideout
// Copyright: 2002-2005  3Dlabs Inc. Ltd.  All rights reserved.
// License:   see 3Dlabs-license.txt
//

#ifndef FRAME_H
#define FRAME_H
#include <wx/wx.h>
#include <wx/wxhtml.h>
class TCanvas;

enum TSection {EHeader, EBody, EFooter, NumSections};

class THtml : public wxHtmlWindow {
  public:
    THtml(wxWindow* parent, int id) : wxHtmlWindow(parent, id) {}
    void OnLinkClicked(const wxHtmlLinkInfo& link);
};

class TFrame : public wxFrame {
  public:
    TFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    void OnFileSave(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);
    void OnHelpAbout(wxCommandEvent& event);
    void OnHelpCommandLine(wxCommandEvent& event);
    void Printf(TSection section, const char* format, ...);
    void Save(const wxString& filename) const;
    void Flush(TSection section);
	TCanvas* GetCanvas() { return canvas; }
  private:
    wxString buffers[NumSections];
    TCanvas* canvas;
    THtml* html[NumSections];
    DECLARE_EVENT_TABLE()
};

#endif
