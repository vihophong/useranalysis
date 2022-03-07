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

#define MAX_N_CH 32
#define N_CH_PLOT 8
#define N_CH_PLOT_H 2
#define N_CH_PLOT_V 4

//#define SLOW_ONLINE 100000

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class MyMainFrame;

class MyMainFrame {
   RQ_OBJECT("MyMainFrame")
private:
   TGMainFrame         *fMain;
   TGMainFrame         *fMain1;
   TGMainFrame         *fMain2;
   TRootEmbeddedCanvas *fEcanvas;
   TCanvas* c1;
   TCanvas* c2;
   TH1F* h1;
   TH1F* h2;
   TH1F* hadc[MAX_N_CH];

   TTimer * timer;
   TTimer * timer2;
   Long_t fupdate_time;
   Long_t fRateCal_time;
   TGTextEntry * autoupdateentry;
   TGTextEntry * ratecalentry;
   TGTextEntry  *PadIdTent;
   TGTextEntry  *ChIDTent;

   TGStatusBar* statusbar;

   TGTextView * DAQstatusTextDialogs;

   TGListBox * listBoxAssignedCh;
   TList* fSelectedAssigedCh;
   int fselectedch[MAX_N_CH];
   int fnselectedch;
   TLegend* fC1LegendOverlay;

   char buf[500];
public:
   MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h);
   virtual ~MyMainFrame();
   TCanvas* GetC1(){return c1;}
   TCanvas* GetC2(){return c2;}
   void SetUpdateTime(Long_t update_time){fupdate_time=update_time;}
   void SetRateCalTime(Long_t RateCal_time){fRateCal_time=RateCal_time;}
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
   void AssignChannel();
   void ClearC1();
   void ClearC2();
   void PlotSpectra();
   void OverlaySpectra();

   ClassDef (MyMainFrame,0);
};

#endif /* __MYMAINFRAME_H__ */
