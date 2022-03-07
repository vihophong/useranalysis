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
#include <fstream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;

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


MyMainFrame::MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h):fhistoconfig()
{
    fupdate_time=1000;
    fRateCal_time=10000;
    for (Int_t i=0;i<MAX_CH_RATEUPDATE;i++) flag_rateupdate[i]=true;
    fMain = new TGMainFrame(p,w,h);
    fMain2 = new TGMainFrame(p,w,h);
    fMain1 = new TGMainFrame(p,w,h);   
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
void MyMainFrame::GetConfig(char *configfile)
{
    //Set Default Config
    std::string line;
    std::ifstream infile(configfile);
    fhistoconfig.n_th1=0;
    fhistoconfig.n_th2=0;
    fhistoconfig.c1_nhist=0;
    fhistoconfig.c2_nhist=0;
    fhistoconfig.n_listbox_entries=0;
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        if (line[0]=='#') continue;
        TString cmd,name_i;iss>>cmd>>name_i;

        if (cmd=="TH1"){
            fhistoconfig.th1_name[fhistoconfig.n_th1]=name_i;
            fhistoconfig.n_th1++;
        }
        if (cmd=="TH2"){
            fhistoconfig.th2_name[fhistoconfig.n_th2]=name_i;
            fhistoconfig.n_th2++;        }

        if (cmd=="CANVAS1"){
            for (Int_t i=0;i<fhistoconfig.n_th1;i++){
                if (name_i==fhistoconfig.th1_name[i]){
                    fhistoconfig.c1_hist_id[fhistoconfig.c1_nhist]=i;
                    fhistoconfig.c1_nhist++;
                }
            }
            for (Int_t i=0;i<fhistoconfig.n_th2;i++){
                if (name_i==fhistoconfig.th2_name[i]){
                    fhistoconfig.c1_hist_id[fhistoconfig.c1_nhist]=i+MAX_N_TH1;
                    fhistoconfig.c1_nhist++;
                }
            }
        }

        if (cmd=="CANVAS2"){
            for (Int_t i=0;i<fhistoconfig.n_th1;i++){
                if (name_i==fhistoconfig.th1_name[i]){
                    fhistoconfig.c2_hist_id[fhistoconfig.c2_nhist]=i;
                    fhistoconfig.c2_nhist++;
                }
            }
            for (Int_t i=0;i<fhistoconfig.n_th2;i++){
                if (name_i==fhistoconfig.th2_name[i]){
                    fhistoconfig.c2_hist_id[fhistoconfig.c2_nhist]=i+MAX_N_TH1;
                    fhistoconfig.c2_nhist++;
                }
            }
        }

        if (cmd=="LISTBOX"){
            for (Int_t i=0;i<fhistoconfig.n_th1;i++){
                if (name_i==fhistoconfig.th1_name[i]){
                    fhistoconfig.listbox_hist_id[fhistoconfig.n_listbox_entries]=i;
                    fhistoconfig.n_listbox_entries++;
                }
            }
            for (Int_t i=0;i<fhistoconfig.n_th2;i++){
                if (name_i==fhistoconfig.th2_name[i]){
                    fhistoconfig.listbox_hist_id[fhistoconfig.n_listbox_entries]=i+MAX_N_TH1;
                    fhistoconfig.n_listbox_entries++;
                }
            }
        }
        if (cmd=="RATE"){
            for (Int_t i=0;i<fhistoconfig.n_th1;i++){
                if (name_i==fhistoconfig.th1_name[i]){
                    fhistoconfig.rate_histo_id=i;
                }
            }
        }
        if (cmd=="RATECAL"){
            for (Int_t i=0;i<fhistoconfig.n_th1;i++){
                if (name_i==fhistoconfig.th1_name[i]){
                    fhistoconfig.ratecal_histo_id=i;
                }
            }
        }
        if (cmd=="RATECALTOSCALE"){
            for (Int_t i=0;i<fhistoconfig.n_th1;i++){
                if (name_i==fhistoconfig.th1_name[i]){
                    fhistoconfig.ratecal_histo_to_scale_id=i;
                }
            }
        }
    }

    //! print
    cout<<"TH1 "<<fhistoconfig.n_th1<<endl;
    for (Int_t i=0;i<fhistoconfig.n_th1;i++)
        cout<<i<<" - "<<fhistoconfig.th1_name[i]<<endl;
    cout<<"TH2 "<<fhistoconfig.n_th2<<endl;
    for (Int_t i=0;i<fhistoconfig.n_th2;i++)
        cout<<i+MAX_N_TH1<<" - "<<fhistoconfig.th2_name[i]<<endl;
    cout<<"C1 "<<fhistoconfig.c1_nhist<<endl;
    for (Int_t i=0;i<fhistoconfig.c1_nhist;i++)
        cout<<fhistoconfig.c1_hist_id[i]<<endl;
    cout<<"C2 "<<fhistoconfig.c2_nhist<<endl;
    for (Int_t i=0;i<fhistoconfig.c2_nhist;i++)
        cout<<fhistoconfig.c2_hist_id[i]<<endl;
    cout<<"LISTBOX "<<fhistoconfig.n_listbox_entries<<endl;
    for (Int_t i=0;i<fhistoconfig.n_listbox_entries;i++)
        cout<<fhistoconfig.listbox_hist_id[i]<<endl;
    cout<<"rate_histo_id "<<fhistoconfig.rate_histo_id<<endl;
    cout<<"ratecal_histo_id "<<fhistoconfig.ratecal_histo_id<<endl;
    cout<<"ratecal_histo_to_scale_id "<<fhistoconfig.ratecal_histo_to_scale_id<<endl;

    cout<<"\n GetConfig DONE!"<<endl;
}

void MyMainFrame::InitHistograms()
{
    for (Int_t i=0;i<fhistoconfig.n_th1;i++)
        fhistoconfig.h1d[i]=(TH1F*) gDirectory->Get(fhistoconfig.th1_name[i].Data());
    for (Int_t i=0;i<fhistoconfig.n_th2;i++)
        fhistoconfig.h2d[i]=(TH2F*) gDirectory->Get(fhistoconfig.th2_name[i].Data());
    cout<<"\n InitHistograms DONE!"<<endl;
}
void MyMainFrame::DrawFrameCanvas()
{
    //!  frame
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
    for (int i = 0; i < fhistoconfig.n_listbox_entries; ++i) {
       if (fhistoconfig.listbox_hist_id[i]<MAX_N_TH1)
           fhistoconfig.listbox_entry[i]=fhistoconfig.h1d[fhistoconfig.listbox_hist_id[i]]->GetTitle();
       else
           fhistoconfig.listbox_entry[i]=fhistoconfig.h2d[fhistoconfig.listbox_hist_id[i]-MAX_N_TH1]->GetTitle();

       listBoxAssignedCh->AddEntry(fhistoconfig.listbox_entry[i].Data(),i+1);
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

    //! 2nd frame

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
    fhistoconfig.c2=new TCanvas("c2","c2",900,600);
    Int_t ncol=fhistoconfig.c2_nhist/2+fhistoconfig.c2_nhist%2;
    fhistoconfig.c2->Divide(ncol,2);
    for (int i=0;i<fhistoconfig.c2_nhist;i++){
        fhistoconfig.c2->cd(i+1);
        if (fhistoconfig.c2_hist_id[i]<MAX_N_TH1){
            fhistoconfig.h1d[fhistoconfig.c2_hist_id[i]]->SetLineColor(4);
            fhistoconfig.h1d[fhistoconfig.c2_hist_id[i]]->Draw();
        }else
            fhistoconfig.h2d[fhistoconfig.c2_hist_id[i]-MAX_N_TH1]->Draw("colz");
    }
    framehisto2->SetEditable(kFALSE);

    //fMain2->Layout();
    // Initialize the layout algorithm
    fMain2->Resize(fMain2->GetDefaultSize());
    // Map main frame
    fMain2->MapWindow();


    //! 3rd frame

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
    fhistoconfig.c1=new TCanvas("c1","c1",1000,600);
    Int_t nrow=fhistoconfig.c1_nhist/2+fhistoconfig.c1_nhist%2;
    fhistoconfig.c1->Divide(2,nrow);
    for (int i=0;i<fhistoconfig.c1_nhist;i++){
        fhistoconfig.c1->cd(i+1);
        if (fhistoconfig.c1_hist_id[i]<MAX_N_TH1){
            fhistoconfig.h1d[fhistoconfig.c1_hist_id[i]]->SetLineColor(4);
            fhistoconfig.h1d[fhistoconfig.c1_hist_id[i]]->Draw();
        }else
            fhistoconfig.h2d[fhistoconfig.c1_hist_id[i]-MAX_N_TH1]->Draw("colz");
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
    fC1LegendOverlay=new TLegend();

    cout<<"\n DrawFrameCanvas DONE!"<<endl;
}

void MyMainFrame::DoCalculateRate() {
    for (int i=0;i<fhistoconfig.h1d[fhistoconfig.ratecal_histo_id]->GetNbinsX();i++){
#ifdef CAL_RATE_DGTZ
        fhistoconfig.h1d[fhistoconfig.rate_histo_id]->SetBinContent(i+1,(double)fhistoconfig.h1d[fhistoconfig.ratecal_histo_id]->GetBinContent(i+1));
#else
        fhistoconfig.h1d[fhistoconfig.rate_histo_id]->SetBinContent(i+1,(double)fhistoconfig.h1d[fhistoconfig.ratecal_histo_id]->GetBinContent(i+1)/(double)fRateCal_time*1000.);
#endif
    }
#ifdef CAL_RATE_DGTZ
    fhistoconfig.h1d[fhistoconfig.rate_histo_id]->Divide(fhistoconfig.h1d[fhistoconfig.ratecal_histo_to_scale_id]);
#endif
    fhistoconfig.h1d[fhistoconfig.ratecal_histo_id]->Reset();
    fhistoconfig.h1d[fhistoconfig.ratecal_histo_to_scale_id]->Reset();
    for (int i=0;i<MAX_CH_RATEUPDATE;i++) flag_rateupdate[i]=true;
}

void MyMainFrame::DoUpdate() {
    for (int ch=0;ch<fhistoconfig.c1_nhist;ch++){
        fhistoconfig.c1->cd(ch+1)->Modified();
        fhistoconfig.c1->cd(ch+1)->Update();
    }
    fhistoconfig.c1->Modified();
    fhistoconfig.c1->Update();

    for (int ch=0;ch<fhistoconfig.c2_nhist;ch++){
        fhistoconfig.c2->cd(ch+1)->Modified();
        fhistoconfig.c2->cd(ch+1)->Update();
    }
    fhistoconfig.c2->Modified();
    fhistoconfig.c2->Update();

    std::string res;
    res=execget("rcdaq_client daq_status 2");
    DAQstatusTextDialogs->LoadBuffer(res.data());
}

void MyMainFrame::DoReset() {
    for (Int_t i=0;i<fhistoconfig.n_th1;i++){
        fhistoconfig.h1d[i]->Reset();
    }
    for (Int_t i=0;i<fhistoconfig.n_th2;i++){
        fhistoconfig.h2d[i]->Reset();
    }
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
    for (int i=0;i<fhistoconfig.c1_nhist;i++){
        if (fhistoconfig.c1_hist_id[i]<MAX_N_TH1)
            fhistoconfig.h1d[fhistoconfig.c1_hist_id[i]]->Reset();
        else
            fhistoconfig.h2d[fhistoconfig.c1_hist_id[i]-MAX_N_TH1]->Reset();
    }
}

void MyMainFrame::ClearC2() {
    for (int i=0;i<fhistoconfig.c2_nhist;i++){
        if (fhistoconfig.c2_hist_id[i]<MAX_N_TH1)
            fhistoconfig.h1d[fhistoconfig.c2_hist_id[i]]->Reset();
        else
            fhistoconfig.h2d[fhistoconfig.c2_hist_id[i]-MAX_N_TH1]->Reset();
    }
}

void MyMainFrame::PlotSpectra() {
    fhistoconfig.c1->Clear();
    fSelectedAssigedCh->Clear();
    listBoxAssignedCh->GetSelectedEntries(fSelectedAssigedCh);
    TIter next(fSelectedAssigedCh);
    TObject* object = 0;
    fhistoconfig.n_selected_listbox_entries=0;
    while((object = next())){
        TGTextLBEntry* te=(TGTextLBEntry*) object;
        te->GetTitle();
        TString selectedTitle(te->GetTitle());        
        for (Int_t i=0;i<fhistoconfig.n_listbox_entries;i++){
            if (selectedTitle==fhistoconfig.listbox_entry[i]){
                fhistoconfig.selected_listbox_hist_id[fhistoconfig.n_selected_listbox_entries]=fhistoconfig.listbox_hist_id[i];
                fhistoconfig.n_selected_listbox_entries++;
            }
        }
    }
    //! copy to existing histo id table (maybe not nice)
    fhistoconfig.c1_nhist=fhistoconfig.n_selected_listbox_entries;
    for (int i=0;i<fhistoconfig.n_selected_listbox_entries;i++)
        fhistoconfig.c1_hist_id[i]=fhistoconfig.selected_listbox_hist_id[i];

    if (fhistoconfig.n_selected_listbox_entries==0) return;
    if (fhistoconfig.n_selected_listbox_entries==1){
        fhistoconfig.c1->cd();
        if (fhistoconfig.selected_listbox_hist_id[0]<MAX_N_TH1){
            fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[0]]->SetLineColor(4);
            fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[0]]->Draw();
        }else
            fhistoconfig.h2d[fhistoconfig.selected_listbox_hist_id[0]-MAX_N_TH1]->Draw("colz");
    }else{
        int nrow=fhistoconfig.n_selected_listbox_entries/2+fhistoconfig.n_selected_listbox_entries%2;
        fhistoconfig.c1->Divide(2,nrow);
        for (int i=0;i<fhistoconfig.n_selected_listbox_entries;i++){
            fhistoconfig.c1->cd(i+1);
            if (fhistoconfig.selected_listbox_hist_id[i]<MAX_N_TH1){
                fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[i]]->SetLineColor(4);
                fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[i]]->Draw();
            }else
                fhistoconfig.h2d[fhistoconfig.selected_listbox_hist_id[i]-MAX_N_TH1]->Draw("colz");
        }
    }
}

void MyMainFrame::OverlaySpectra() {
    fhistoconfig.c1->Clear();
    fSelectedAssigedCh->Clear();
    listBoxAssignedCh->GetSelectedEntries(fSelectedAssigedCh);
    TIter next(fSelectedAssigedCh);
    TObject* object = 0;
    fhistoconfig.n_selected_listbox_entries=0;
    while((object = next())){
        TGTextLBEntry* te=(TGTextLBEntry*) object;
        te->GetTitle();
        TString selectedTitle(te->GetTitle());
        for (Int_t i=0;i<fhistoconfig.n_listbox_entries;i++){
            if (selectedTitle==fhistoconfig.listbox_entry[i]){
                fhistoconfig.selected_listbox_hist_id[fhistoconfig.n_selected_listbox_entries]=fhistoconfig.listbox_hist_id[i];
                fhistoconfig.n_selected_listbox_entries++;
            }
        }
    }
    //! copy to existing histo id table (maybe not nice)
    fhistoconfig.c1_nhist=fhistoconfig.n_selected_listbox_entries;
    for (int i=0;i<fhistoconfig.n_selected_listbox_entries;i++)
        fhistoconfig.c1_hist_id[i]=fhistoconfig.selected_listbox_hist_id[i];

    if (fhistoconfig.n_selected_listbox_entries==0) return;

    fhistoconfig.c1->cd();
    fC1LegendOverlay->Clear();
    double Max_histo_height=0;
    if (fhistoconfig.selected_listbox_hist_id[0]<MAX_N_TH1){
        fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[0]]->SetLineColor(1);
        fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[0]]->Draw();
        if (fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[0]]->GetMaximum()>Max_histo_height) Max_histo_height=fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[0]]->GetMaximum();
        fC1LegendOverlay->AddEntry(fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[0]],fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[0]]->GetTitle());
    }else{
        fhistoconfig.h2d[fhistoconfig.selected_listbox_hist_id[0]-MAX_N_TH1]->Draw("colz");
        fC1LegendOverlay->AddEntry(fhistoconfig.h2d[fhistoconfig.selected_listbox_hist_id[0]-MAX_N_TH1],fhistoconfig.h2d[fhistoconfig.selected_listbox_hist_id[0]-MAX_N_TH1]->GetTitle());
    }


    Int_t colormap[]={2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,24,25,26,27,28,29,30,31,32};    
    for (int i=1;i<fhistoconfig.n_selected_listbox_entries;i++){
        if (fhistoconfig.selected_listbox_hist_id[i]<MAX_N_TH1){
            fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[i]]->SetLineColor(colormap[i]);
            fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[i]]->Draw("same");
            if (fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[i]]->GetMaximum()>Max_histo_height) Max_histo_height=fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[i]]->GetMaximum();
            fC1LegendOverlay->AddEntry(fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[i]],fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[i]]->GetTitle());
        }else{
            fhistoconfig.h2d[fhistoconfig.selected_listbox_hist_id[i]-MAX_N_TH1]->Draw("same");
            fC1LegendOverlay->AddEntry(fhistoconfig.h2d[fhistoconfig.selected_listbox_hist_id[i]-MAX_N_TH1],fhistoconfig.h2d[fhistoconfig.selected_listbox_hist_id[i]-MAX_N_TH1]->GetTitle());
        }
    }
    if (fhistoconfig.selected_listbox_hist_id[0]<MAX_N_TH1){
        if (Max_histo_height>0) fhistoconfig.h1d[fhistoconfig.selected_listbox_hist_id[0]]->GetYaxis()->SetRangeUser(0,Max_histo_height+Max_histo_height*0.2);
    }

    //! tlegend
    fC1LegendOverlay->Draw();
}

