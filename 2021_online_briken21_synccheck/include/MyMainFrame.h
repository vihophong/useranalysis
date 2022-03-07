#ifndef __MYMAINFRAME_H__
#define __MYMAINFRAME_H__

#include <TQObject.h>
#include <RQ_OBJECT.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TTimer.h>
#include <TGTextEntry.h>
#include <TGTextView.h>
#include <TGTextEdit.h>
#include <TGText.h>
#include <TGLabel.h>
#include <TGStatusBar.h>
#include <TGListBox.h>
#include <TLegend.h>

#include <string>

#define CAL_RATE_DGTZ 1

#define MAX_N_TH1 32
#define MAX_N_TH2 32
#define MAX_N_ALL 64

#define MAX_CH_RATEUPDATE 100

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class MyMainFrame;

typedef struct HistogramsConfig{
    TString th1_name[MAX_N_TH1];
    TString th2_name[MAX_N_TH2];
    Int_t n_th1;
    Int_t n_th2;

    Int_t c1_hist_id[MAX_N_ALL];
    Int_t c1_nhist;

    Int_t c2_hist_id[MAX_N_ALL];//negative
    Int_t c2_nhist;

    TString listbox_entry[MAX_N_ALL];
    Int_t listbox_hist_id[MAX_N_ALL];
    Int_t n_listbox_entries;

    Int_t selected_listbox_hist_id[MAX_N_ALL];
    Int_t n_selected_listbox_entries;

    Int_t rate_histo_id;
    Int_t ratecal_histo_id;
    Int_t ratecal_histo_to_scale_id;

    TCanvas* c1;
    TCanvas* c2;
    TH1F* h1d[MAX_N_TH1];
    TH2F* h2d[MAX_N_TH2];
}HistogramsConfig_t;

class MyMainFrame {
   RQ_OBJECT("MyMainFrame")
private:
   TGMainFrame         *fMain;
   TGMainFrame         *fMain1;
   TGMainFrame         *fMain2;
   TRootEmbeddedCanvas *fEcanvas;

   TTimer * timer;
   TTimer * timer2;
   Long_t fupdate_time;
   Long_t fRateCal_time;
   Bool_t flag_rateupdate[MAX_CH_RATEUPDATE];

   TGTextEntry * autoupdateentry;
   TGTextEntry * ratecalentry;
   TGTextEntry  *PadIdTent;
   TGTextEntry  *ChIDTent;

   TGStatusBar* statusbar;

   TGTextView * DAQstatusTextDialogs;

   TGListBox * listBoxAssignedCh;
   TList* fSelectedAssigedCh;    
   TLegend* fC1LegendOverlay;

   char buf[500];

   HistogramsConfig_t fhistoconfig;
public:
   MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h);
   virtual ~MyMainFrame();
   void GetConfig(char* configfile=(char *)"histosconfig.txt");
   void InitHistograms();
   void DrawFrameCanvas();
   TCanvas* GetC1(){return fhistoconfig.c1;}
   TCanvas* GetC2(){return fhistoconfig.c2;}
   void SetUpdateTime(Long_t update_time){fupdate_time=update_time;}
   void SetRateCalTime(Long_t RateCal_time){fRateCal_time=RateCal_time;}
   Bool_t GetFlagRateUpdate(int ch){return flag_rateupdate[ch];}
   void ResetFlagRateUpdate(int ch){flag_rateupdate[ch]=false;}
   TGMainFrame* GetMainFrame(){return fMain;}
   void DoReset();
   void DoUpdate();
   void DoCalculateRate();
   void DoOpenDAQ();
   void DoBeginDAQ();
   void DoEndDAQ();
   void DoDAQConfig();
   void StopOnline();
   void ResumeOnline();
   void SetTimer();
   void SetRateCalTime();   
   void ClearC1();
   void ClearC2();
   void PlotSpectra();
   void OverlaySpectra();
   ClassDef (MyMainFrame,0);
};

#endif /* __MYMAINFRAME_H__ */
