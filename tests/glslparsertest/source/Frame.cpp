//
// Author:    Philip Rideout
// Copyright: 2002-2005  3Dlabs Inc. Ltd.  All rights reserved.
// License:   see 3Dlabs-license.txt
//

#include <wx/wx.h>
#include <wx/statline.h>
#include <wx/wxhtml.h>
#include "App.h"
#include "Frame.h"
#include "Canvas.h"
#include "Tables.h"
#include <wx/file.h>

BEGIN_EVENT_TABLE(TFrame, wxFrame)
    EVT_SIZE(TFrame::OnSize)
    EVT_MENU(Id::FileSave, TFrame::OnFileSave)
    EVT_MENU(Id::FileExit, TFrame::OnFileExit)
    EVT_MENU(Id::HelpCommandLine, TFrame::OnHelpCommandLine)
    EVT_MENU(Id::HelpAbout, TFrame::OnHelpAbout)
END_EVENT_TABLE()

TFrame::TFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : canvas(0), wxFrame(0, -1, title, pos, size, wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN)
{
    wxGetApp().SetFrame(this);

    canvas = new TCanvas(this);
    wxBoxSizer* vertical = new wxBoxSizer(wxVERTICAL);
    vertical->Add(canvas, 1, wxALIGN_CENTRE_VERTICAL | wxEXPAND | wxALL, 1);

    html[EHeader] = new THtml(this, Id::WidgetHtml);
    vertical->Add(html[EHeader], 6, wxALIGN_CENTRE_VERTICAL | wxEXPAND | wxALL);

    html[EBody] = new THtml(this, Id::WidgetHtml);
    vertical->Add(html[EBody], 20, wxALIGN_CENTRE_VERTICAL | wxEXPAND | wxALL, 1);

    html[EFooter] = new THtml(this, Id::WidgetHtml);
    vertical->Add(html[EFooter], 8, wxALIGN_CENTRE_VERTICAL | wxEXPAND | wxALL);

    wxMenu* menu;
    wxMenuBar* menuBar = new wxMenuBar;

    menu = new wxMenu;
    menu->Append(Id::FileSave, "Save results...\tCtrl+S");
    menu->Append(Id::FileExit, "Exit\tQ");
    menuBar->Append(menu, "File");

    menu = new wxMenu;
    menu->Append(Id::HelpCommandLine, "Command line options...");
    menu->Append(Id::HelpAbout, "About...");
    menuBar->Append(menu, "Help");

    SetMenuBar(menuBar);
    SetSizer(vertical);
    SetIcon(wxIcon("CHECKMARK_ICON", wxBITMAP_TYPE_ICO_RESOURCE));
}

void TFrame::OnHelpCommandLine(wxCommandEvent& event)
{
    wxCmdLineParser parser(TApp::CommandLineDescription);
    parser.Usage();
}

void TFrame::OnHelpAbout(wxCommandEvent& event)
{
    wxMessageBox("GLSL Parser Test v1.8\nCopyright © 2005 3Dlabs.  All rights reserved.\n\nglslparsertest: a tool for verifying the correctness of your GLSL parser.\n\nFor questions and feedback, go to http://www.3dlabs.com/contact.\n\nThanks to Jordan Russell for InnoSetup.", "about glslparsertest", wxOK | wxICON_INFORMATION, this);
}

void TFrame::OnFileExit(wxCommandEvent& event)
{
    Close();
    event.Skip();
}

void TFrame::OnFileSave(wxCommandEvent& event)
{
    wxString filename = wxFileSelector("Save as", "..", wxGetApp().LogFile(), "html", "HTML (*.html)|*.html", wxSAVE);
    if (!filename.empty())
        Save(filename);
}

void TFrame::Save(const wxString& filename) const
{
    wxFile output(filename, wxFile::write);
    output.Write(html[EHeader]->GetParser()->GetSource()->c_str());
    output.Write("<html><body><br></body></html>\n");
    output.Write(html[EFooter]->GetParser()->GetSource()->c_str());
    output.Write("<html><body><p><hr></body></html>\n");
    output.Write(html[EBody]->GetParser()->GetSource()->c_str());
}

void TFrame::Printf(TSection section, const char* format, ...)
{
    wxString message;

    va_list marker;
    va_start(marker, format);
    message.PrintfV(format, marker);
    buffers[section] += message;
}

void TFrame::Flush(TSection section)
{
    wxString content = "<html><body bgcolor=#ffffff>\n" + buffers[section];

    if (section == EBody)
        content = content + "</table>\n";

    content = content + "\n</body></html>";

    html[section]->SetPage(content);
    html[section]->Refresh(false);
    html[section]->Update();
}

void THtml::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	TFrame* frame = static_cast<TFrame*>(GetParent());
	TCanvas* canvas = frame->GetCanvas();
    const TTest* test = canvas->Find(link.GetHref());

    if (!test || test->actual == Crash)
        TCanvas::ShowDialog(link.GetHref(), "infolog is unavailable", 0);
    else
        canvas->Compile(link.GetHref(), true);
}
