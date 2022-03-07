#include "MyMainFrame.h"
#include <TGClient.h>
#include "TSystem.h"
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>

#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TROOT.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>

#include <TDirectory.h>
#include <iostream>


#include <iostream>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string execget(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    auto pipe = popen(cmd, "r");

    if (!pipe) throw std::runtime_error("popen() failed!");

    while (!feof(pipe))
    {
        if (fgets(buffer.data(), 128, pipe) != nullptr)
            result += buffer.data();
    }

    auto rc = pclose(pipe);

//    if (rc == EXIT_SUCCESS)
//    {
//        std::cout << "SUCCESS\n";
//    }
//    else
//    {
//        std::cout << "FAILED\n";
//    }

    return result;
}


MyMainFrame::MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h) {
    fupdate_time=1000;
    fRateCal_time=10000;

    h1=(TH1F*) gDirectory->Get("h1");
    h2=(TH1F*) gDirectory->Get("h2");
    for (Int_t i=0;i<MAX_N_CH;i++){
        hadc[i]=(TH1F*) gDirectory->Get(Form("hadc%d",i));
     }

    // Create a main frame
    fMain = new TGMainFrame(p,w,h);    

    // Create a horizontal frame widget with buttons
    TGGroupFrame* hframe = new TGGroupFrame(fMain, "DAQ controls", kHorizontalFrame);
    TGTextButton *daqconfigbtn = new TGTextButton(hframe,"&Config DAQ");
    daqconfigbtn->Connect("Clicked()","MyMainFrame",this,"DoDAQConfig()");
    hframe->AddFrame(daqconfigbtn, new TGLayoutHints(kLHintsCenterX,
                                                   5,5,3,4));
    TGTextButton *daqopenbtn = new TGTextButton(hframe,"&Open File");
    daqopenbtn->Connect("Clicked()","MyMainFrame",this,"DoOpenDAQ()");
    hframe->AddFrame(daqopenbtn, new TGLayoutHints(kLHintsCenterX,
                                                   5,5,3,4));
    TGTextButton *daqbeginbtn = new TGTextButton(hframe,"&Begin Run");
    daqbeginbtn->Connect("Clicked()","MyMainFrame",this,"DoBeginDAQ()");
    hframe->AddFrame(daqbeginbtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *daqendbtn = new TGTextButton(hframe,"&End Run");
    daqendbtn->Connect("Clicked()","MyMainFrame",this,"DoEndDAQ()");
    hframe->AddFrame(daqendbtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *exit = new TGTextButton(hframe,"&Exit",
                                 "gApplication->Terminate(0)");
    hframe->AddFrame(exit, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));
    // Create canvas widget
//    fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fMain,900,700);
//    fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX |
//                    kLHintsExpandY, 10,10,10,1));

    // Create bottom frame for reset button    
    TGGroupFrame* hframe2 = new TGGroupFrame(fMain, "Online Controls", kHorizontalFrame);
    TGTextButton *stoponlinebtn = new TGTextButton(hframe2,"&Pause Online");
    stoponlinebtn->Connect("Clicked()","MyMainFrame",this,"StopOnline()");
    hframe2->AddFrame(stoponlinebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *resumeonlinebtn = new TGTextButton(hframe2,"&Resume Online");
    resumeonlinebtn->Connect("Clicked()","MyMainFrame",this,"ResumeOnline()");
    hframe2->AddFrame(resumeonlinebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *resetbtn = new TGTextButton(hframe2,"&Clear Histograms");
    resetbtn->Connect("Clicked()","MyMainFrame",this,"DoReset()");
    hframe2->AddFrame(resetbtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));

    fMain->AddFrame(hframe2, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));

    // Create bottom frame for reset button
    TGGroupFrame* hframe3 = new TGGroupFrame(fMain, "Timer Settings", kHorizontalFrame);
    autoupdateentry = new TGTextEntry(hframe3,"1000");
    hframe3->AddFrame(autoupdateentry, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));

    TGTextButton *autoupdatebtn = new TGTextButton(hframe3,"&Set AutoUpdate Timer");
    autoupdatebtn->Connect("Clicked()","MyMainFrame",this,"SetTimer()");
    hframe3->AddFrame(autoupdatebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));

    ratecalentry = new TGTextEntry(hframe3,"10000");
    hframe3->AddFrame(ratecalentry, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *setratecaltimebtn = new TGTextButton(hframe3,"&Set Rate Cal. Timer");
    setratecaltimebtn->Connect("Clicked()","MyMainFrame",this,"SetRateCalTime()");
    hframe3->AddFrame(setratecaltimebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    fMain->AddFrame(hframe3, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));


    //! Assign channel to monitor
    TGGroupFrame* grframe = new TGGroupFrame(fMain, "Assign DAQ Monitor 1", kHorizontalFrame);
    grframe->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    // 2 column, n rows
    grframe->SetLayoutManager(new TGMatrixLayout(grframe, 0, 2, 10));

    listBoxAssignedCh = new TGListBox(grframe, 89);
    fSelectedAssigedCh = new TList;
    char tmp[50];
    for (int i = 0; i < MAX_N_CH; ++i) {
       sprintf(tmp, "Channel %i", i);
       listBoxAssignedCh->AddEntry(tmp, i+1);
    }
    listBoxAssignedCh->Resize(200,150);
    listBoxAssignedCh->SetMultipleSelections(kTRUE);
    grframe->AddFrame(listBoxAssignedCh, new TGLayoutHints(kLHintsTop | kLHintsLeft |
                                         kLHintsExpandX | kLHintsExpandY,
                                         5, 5, 5, 5));

    TGVerticalFrame *assignoverlaybtnframe = new TGVerticalFrame(grframe, 200, 100);

    TGTextButton *plotC1btn = new TGTextButton(assignoverlaybtnframe,"&Plot Spectra");
    plotC1btn->Connect("Clicked()","MyMainFrame",this,"PlotSpectra()");
    assignoverlaybtnframe->AddFrame(plotC1btn,new TGLayoutHints(kLHintsTop | kLHintsLeft |
                                                       kLHintsExpandX | kLHintsExpandY,
                                                       5, 5, 5, 5));

    TGTextButton *overlayC1btn = new TGTextButton(assignoverlaybtnframe,"&OverLay Spectra");
    overlayC1btn->Connect("Clicked()","MyMainFrame",this,"OverlaySpectra()");
    assignoverlaybtnframe->AddFrame(overlayC1btn,new TGLayoutHints(kLHintsTop | kLHintsLeft |
                                                       kLHintsExpandX | kLHintsExpandY,
                                                       5, 5, 5, 5));

    grframe->AddFrame(assignoverlaybtnframe,new TGLayoutHints(kLHintsTop | kLHintsLeft |
                                                       kLHintsExpandX | kLHintsExpandY,
                                                       5, 5, 5, 5));

    fMain->AddFrame(grframe, new TGLayoutHints(kLHintsCenterX,
                                             2,2,2,2));

//    grframe->AddFrame(new TGLabel(grframe, new TGHotString("Pad to be assigned: ")));

//    TGTextBuffer *tbuf = new TGTextBuffer(10);
//    tbuf->AddText(0, "1");
//    PadIdTent = new TGTextEntry(grframe, tbuf);
//    PadIdTent->Resize(50, PadIdTent->GetDefaultHeight());
//    grframe->AddFrame(PadIdTent);

//    grframe->AddFrame(new TGLabel(grframe, new TGHotString("Channel to be assigned")));

//    TGTextBuffer *tbuf2 = new TGTextBuffer(10);
//    tbuf2->AddText(0, "0");
//    ChIDTent = new TGTextEntry(grframe, tbuf2);
//    ChIDTent->Resize(50, ChIDTent->GetDefaultHeight());
//    grframe->AddFrame(ChIDTent);

//    fMain->AddFrame(grframe, new TGLayoutHints(kLHintsCenterX,
//                                              2,2,2,2));
//    TGHorizontalFrame *hframe4 = new TGHorizontalFrame(fMain,900,20);
//    TGTextButton *assignchbutton = new TGTextButton(hframe4,"&Assign Pad to Channel");
//    assignchbutton->Connect("Clicked()","MyMainFrame",this,"AssignChannel()");
//    hframe4->AddFrame(assignchbutton);
//    fMain->AddFrame(hframe4, new TGLayoutHints(kLHintsCenterX,
//                                              2,2,2,2));

    TGGroupFrame* grframestatus = new TGGroupFrame(fMain, "DAQ Status", kHorizontalFrame);
    DAQstatusTextDialogs = new TGTextView(grframestatus,800,200, kSunkenFrame | kDoubleBorder);
    DAQstatusTextDialogs->LoadBuffer("checking...");
    grframestatus->AddFrame(DAQstatusTextDialogs);
    fMain->AddFrame(grframestatus, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));

    // Create status frame containing a button and a text entry widget
    statusbar = new TGStatusBar(fMain, 900, 20, kVerticalFrame);
    Int_t parts[] = {45, 15, 10, 30};
    statusbar->SetParts(parts, 4);
    statusbar->Draw3DCorner(kFALSE);
    fMain->AddFrame(statusbar, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));

    // Set a name to the main frame
    fMain->SetWindowName("DAQ ONLINE CONTROLS");

    // Map all subwindows of main frame
    fMain->MapSubwindows();

    // Initialize the layout algorithm
    fMain->Resize(fMain->GetDefaultSize());

    // Map main frame
    fMain->MapWindow();


    // ------------------------------------
    // Create a main frame 2 for histograms
    fMain2 = new TGMainFrame(p,w,h);

    TGHorizontalFrame *framehisto2 = new TGHorizontalFrame(fMain2, 800, 600);
    fMain2->AddFrame(framehisto2, new TGLayoutHints(kLHintsCenterX,
                                            2,2,2,2));

    TGHorizontalFrame *frameclearhist2 = new TGHorizontalFrame(fMain2, 800, 600);
    TGTextButton* fResetCanvas2btn = new TGTextButton(frameclearhist2, "&Clear Histograms");
    fResetCanvas2btn->Connect("Clicked()","MyMainFrame",this,"ClearC2()");
    frameclearhist2->AddFrame(fResetCanvas2btn, new TGLayoutHints(kLHintsCenterX,
                                                    5,5,3,4));

    fMain2->AddFrame(frameclearhist2,new TGLayoutHints(kLHintsCenterX,
                                                      2,2,2,2));
    // Set a name to the main frame
    fMain2->SetWindowName("DAQ Monitor 2");
    // Map all subwindows of main frame
    fMain2->MapSubwindows();

    // this is where embedding is done:
    framehisto2->SetEditable();
    c2=new TCanvas("c2","c2",900,600);
    c2->Divide(1,2);
    c2->cd(1);
    h1->SetFillColor(2);
    h1->Draw();
    c2->cd(2);
    h2->Draw("colz");
    framehisto2->SetEditable(kFALSE);

    //fMain2->Layout();
    // Initialize the layout algorithm
    fMain2->Resize(fMain2->GetDefaultSize());
    // Map main frame
    fMain2->MapWindow();


    // ------------------------------------
    // Create a main frame 2 for histograms
    fMain1 = new TGMainFrame(p,w,h);

    TGHorizontalFrame *framehisto1 = new TGHorizontalFrame(fMain1, 1200, 800);
    fMain1->AddFrame(framehisto1, new TGLayoutHints(kLHintsCenterX,
                                            2,2,2,2));

    TGHorizontalFrame *frameclearhist1 = new TGHorizontalFrame(fMain1, 1200, 800);
    TGTextButton* fResetCanvas1btn = new TGTextButton(frameclearhist1, "&Clear Histograms");
    fResetCanvas1btn->Connect("Clicked()","MyMainFrame",this,"ClearC1()");
    frameclearhist1->AddFrame(fResetCanvas1btn, new TGLayoutHints(kLHintsCenterX,
                                                    5,5,3,4));

    fMain1->AddFrame(frameclearhist1,new TGLayoutHints(kLHintsCenterX,
                                                      2,2,2,2));
    // Set a name to the main frame
    fMain1->SetWindowName("DAQ Monitor 1");
    // Map all subwindows of main frame
    fMain1->MapSubwindows();

    // this is where embedding is done:
    framehisto1->SetEditable();
    c1=new TCanvas("c1","c1",1000,600);
    c1->Divide(N_CH_PLOT_H,N_CH_PLOT_V);
    fnselectedch=0;
    for (Int_t i=0;i<N_CH_PLOT;i++){
        c1->cd(i+1);
        hadc[i]->Draw();
        fselectedch[fnselectedch]=i;
        fnselectedch++;
    }
    framehisto1->SetEditable(kFALSE);

    //fMain1->Layout();
    // Initialize the layout algorithm
    fMain1->Resize(fMain1->GetDefaultSize());
    // Map main frame
    fMain1->MapWindow();



    //! TIMER
    timer = new TTimer();
    timer->Connect("Timeout()", "MyMainFrame", this, "DoUpdate()");
    timer->Start(fupdate_time, kFALSE);   // 2 seconds single-shot

    timer2 = new TTimer();
    timer2->Connect("Timeout()", "MyMainFrame", this, "DoCalculateRate()");
    timer2->Start(fRateCal_time, kFALSE);   // 2 seconds single-shot

    //! C1 tlegend for Overlay
    //fC1LegendOverlay=new TLegend(0.1,0.7,0.48,0.9);
    fC1LegendOverlay=new TLegend();
    //fC1LegendOverlay->SetLineColor(0);
}
MyMainFrame::~MyMainFrame() {
    // Clean up used widgets: frames, buttons, layout hints
    fMain->Cleanup();
    delete fMain;
    fMain1->Cleanup();
    delete fMain1;
    fMain2->Cleanup();
    delete fMain2;
}

void MyMainFrame::DoCalculateRate() {
    TH1F* h1tmp=(TH1F*) gDirectory->Get("h1tmp");
    for (int i=0;i<h1tmp->GetNbinsX();i++){
        h1->SetBinContent(i+1,(double)h1tmp->GetBinContent(i+1)/(double)fRateCal_time*1000.);
    }
    h1tmp->Reset();
}

void MyMainFrame::DoUpdate() {
    for (int ch=0;ch<N_CH_PLOT;ch++){
        c1->cd(ch+1)->Modified();
        c1->cd(ch+1)->Update();
    }
    c1->Modified();
    c1->Update();

    c2->cd(1)->Modified();
    c2->cd(1)->Update();
    c2->cd(2)->Modified();
    c2->cd(2)->Update();

    std::string res;
    res=execget("rcdaq_client daq_status -ll");
    DAQstatusTextDialogs->LoadBuffer(res.data());
}


void MyMainFrame::AssignChannel() {
    int padid=atoi(PadIdTent->GetText());
    int chid=atoi(ChIDTent->GetText());
    if (padid<=0||padid>N_CH_PLOT) return;
    if (chid>=MAX_N_CH) return;
    TH1F* h=(TH1F*) gDirectory->Get(Form("hadc%d",chid));
    c1->cd(padid);
    h->Draw();
    sprintf(buf,"Assigned channel spectrum %d to Pad %d",chid,padid);
    statusbar->SetText(buf,0);
}

void MyMainFrame::DoReset() {
    for (Int_t i=0;i<MAX_N_CH;i++){
        hadc[i]->Reset();
    }
    h1->Reset();
    h2->Reset();
    sprintf(buf,"Histograms reset");
    statusbar->SetText(buf,0);
}

void MyMainFrame::StopOnline() {
    gROOT->ProcessLine("pstop()");
    sprintf(buf,"Stopped ONLINE");
    statusbar->SetText(buf,0);
}
void MyMainFrame::ResumeOnline(){
    gROOT->ProcessLine("pstart()");
    sprintf(buf,"Resumed ONLINE");
    statusbar->SetText(buf,0);
}

void MyMainFrame::DoBeginDAQ() {
    std::string res=execget("rcdaq_client daq_begin");
    statusbar->SetText(res.data(),0);
}
void MyMainFrame::DoEndDAQ() {
    std::string res=execget("rcdaq_client daq_end");
    statusbar->SetText(res.data(),0);
}
void MyMainFrame::DoOpenDAQ(){
    std::string res=execget("rcdaq_client daq_open");
    sprintf(buf,"DAQ configured");
    statusbar->SetText(buf,0);
}
void MyMainFrame::DoDAQConfig() {
    std::string res=execget("./daqconfig1.sh");
    sprintf(buf,"DAQ configured");
    statusbar->SetText(buf,0);
    statusbar->SetText(res.data(),0);
}

void MyMainFrame::SetTimer() {
    fupdate_time=atoi(autoupdateentry->GetText());        
    timer->SetTime(fupdate_time);
    sprintf(buf,"Set auto update every %d ms",(int)fupdate_time);
    statusbar->SetText(buf,0);
}

void MyMainFrame::SetRateCalTime() {
    fRateCal_time=atoi(ratecalentry->GetText());
    timer2->SetTime(fRateCal_time);
    sprintf(buf,"Set Time for Rate calculator of %d ms",(int)fRateCal_time);
    statusbar->SetText(buf,0);
}


void MyMainFrame::ClearC1() {
    for (int i=0;i<fnselectedch;i++){
        hadc[fselectedch[i]]->Reset();
    }
}

void MyMainFrame::ClearC2() {
    h1->Reset();
    h2->Reset();
}


void MyMainFrame::PlotSpectra() {
    if(fnselectedch==0) return;
    c1->Clear();
    fSelectedAssigedCh->Clear();
    listBoxAssignedCh->GetSelectedEntries(fSelectedAssigedCh);
    TIter next(fSelectedAssigedCh);
    TObject* object = 0;
    fnselectedch=0;
    while((object = next())){
        TGTextLBEntry* te=(TGTextLBEntry*) object;
        te->GetTitle();
        std::string tmpstr(te->GetTitle());
        std::string tmpstr2=tmpstr.substr(8,tmpstr.length()-8);
        fselectedch[fnselectedch]=atoi(tmpstr2.c_str());
        fnselectedch++;
    }
    if (fnselectedch==1){
        c1->cd();
        hadc[fselectedch[0]]->Draw();
    }else{
        int nrow=fnselectedch/2+fnselectedch%2;
        c1->Divide(2,nrow);
        for (int i=0;i<fnselectedch;i++){
            c1->cd(i+1);
            hadc[fselectedch[i]]->SetLineColor(4);
            hadc[fselectedch[i]]->Draw();
        }
    }
}


void MyMainFrame::OverlaySpectra() {
    if(fnselectedch==0) return;
    c1->Clear();
    fSelectedAssigedCh->Clear();
    listBoxAssignedCh->GetSelectedEntries(fSelectedAssigedCh);
    TIter next(fSelectedAssigedCh);
    TObject* object = 0;
    fnselectedch=0;
    while((object = next())){
        TGTextLBEntry* te=(TGTextLBEntry*) object;
        te->GetTitle();
        std::string tmpstr(te->GetTitle());
        std::string tmpstr2=tmpstr.substr(8,tmpstr.length()-8);
        fselectedch[fnselectedch]=atoi(tmpstr2.c_str());
        fnselectedch++;
    }

    c1->cd();
    fC1LegendOverlay->Clear();
    hadc[fselectedch[0]]->SetLineColor(1);
    hadc[fselectedch[0]]->Draw();
    Int_t colormap[]={2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,24,25,26,27,28,29,30,31,32};
    fC1LegendOverlay->AddEntry(hadc[fselectedch[0]],hadc[fselectedch[0]]->GetTitle());
    for (int i=1;i<fnselectedch;i++){
        hadc[fselectedch[i]]->SetLineColor(colormap[i]);
        hadc[fselectedch[i]]->Draw("same");
        fC1LegendOverlay->AddEntry(hadc[fselectedch[i]],hadc[fselectedch[i]]->GetTitle());
    }

    //! tlegend
    fC1LegendOverlay->Draw();

}

