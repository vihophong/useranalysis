#include <iostream>
#include <pmonitor/pmonitor.h>
#include "myanalysis.h"
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
#include <iostream>
#include <fstream>
#include<stdlib.h>
#include<string.h>

#include "TTimer.h"
#include "MyMainFrame.h"
#include <time.h>
//#define DEBUG

#define TDC_PACKET 51

#define COUNTS_TO_UPDATE 1


#define OVER_FLOW 4096


using namespace std;

int init_done = 0;

MyMainFrame* mf;
TH1F *h1;
TH1F *h1tmp;
TH1F *hadc[MAX_N_CH];
TH2F *h2;

Int_t ch_thr[MAX_N_CH];
Int_t ch_ecal0[MAX_N_CH];
Int_t ch_ecal1[MAX_N_CH];

int timeval_subtract (struct timespec *result, struct timespec* x, struct timespec*y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_nsec < y->tv_nsec) {
    int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
    y->tv_nsec -= 1000000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_nsec - y->tv_nsec > 1000000000) {
    int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000;
    y->tv_nsec += 1000000000 * nsec;
    y->tv_sec -= nsec;
  }
  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;
  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

int pinit()
{
    if (init_done) return 1;
    init_done = 1;

    std::ifstream inpf("channel_calib.txt");
      if (inpf.fail()){
        cout<<"No Configuration table is given"<<endl;
        return 1;
      }

      cout<<"Start reading calibration table channel_calib.txt"<<endl;
      Int_t channel;
      Double_t threshold,ecal0,ecal1;
      Int_t mm=0;

      while (inpf.good()){
        inpf>>channel>>threshold>>ecal0>>ecal1;
        cout<<channel<<"-"<<threshold<<"-"<<ecal0<<"-"<<ecal1<<endl;
        ch_ecal0[channel]=ecal0;
        ch_ecal1[channel]=ecal1;
        ch_thr[channel]=threshold;
        mm++;
      }

    h1 = new TH1F ( "h1","channel rate", MAX_N_CH, 0, MAX_N_CH);
    h1tmp = new TH1F ( "h1tmp","histogram of total counts (for rate calculation)", MAX_N_CH, 0, MAX_N_CH);
    for (Int_t i=0;i<MAX_N_CH;i++)
        hadc[i]=new TH1F (Form("hadc%d",i),Form("Channel %d ADC spectrum",i), 5000, 0, 5000);
    h2 = new TH2F ( "h2","All channels: E vs ch",MAX_N_CH,0,MAX_N_CH,5000, 0, 5000);

    mf=new MyMainFrame(gClient->GetRoot(),200,200);

    return 0;
}

int process_event (Event * e)
{
    Packet *adcp = e->getPacket(TDC_PACKET);
    if(adcp){
#ifdef SLOW_ONLINE
        usleep(SLOW_ONLINE);
#endif
        int* temp;
        int* gg;
        gg=(int*) adcp->getIntArray(temp);
        int size=adcp->getPadding();        
        int last_daq_blkcounter = gg[0];
        int last_recieved_trigger= gg[1];
        int tsec= gg[2];
        int tnsec= gg[3];
        int now_daq_ncounter=0;

        bool flag_end_v792counter = false;
        int nwordqdc = 0;
        int lastcounter=0;

        bool flag_not_valid_dantum = false;
#ifdef DEBUG
        cout<<std::dec<<"event last_daq_blkcounter="<<last_daq_blkcounter<<" - last_recieved_trigger="<<last_recieved_trigger<<" -ts(s)="<<tsec<<" - -ts(ns)="<<tnsec<<endl;
#endif
        double t_s=tsec+tnsec*1e-9;
        double rate = (last_recieved_trigger+1)/t_s;
        //cout<<rate<<endl;
        int nadcdatum=0;
        for (int i=4;i<size;i++){
            if((gg[i]&0xF8000000)>>27==0x13){
#ifdef DEBUG
                cout<<std::hex<<"Separation bit counter met 0x"<<gg[i]<<std::dec<<endl;
#endif
                flag_end_v792counter=true;
                //break;
            }
            if (!flag_end_v792counter){//data bit of V726
                //! read qdc      
#ifdef DEBUG
                cout<<"qdc bit"<<std::hex<<"0x"<<((gg[i]&0x7000000)>>24)<<" - 0x"<<gg[i]<<std::dec<<" ch="<<((gg[i]>>16)&0x1F)<<" - adc"<<(gg[i]&0xFFF)<<endl;
#endif
                if (((gg[i]&0x7000000)>>24)==0x2){//Header met                    
                    int qdcnmemorisech=(int)((gg[i]&0x3F00)>>8);
#ifdef DEBUG
                    cout<<"QDC memoriesed channels: "<<qdcnmemorisech<<endl;
#endif
                }else if((!flag_not_valid_dantum)&&(((gg[i]&0x7000000)>>24)==0x0)){
#ifdef DEBUG
                cout<<"WRITE QDC: qdc bit 0x"<<std::hex<<gg[i]<<" msb = 0x"<<((gg[i]&0x7000000)>>24)<<std::dec<<" ch="<<((gg[i]>>16)&0x1F)<<" - adc"<<(gg[i]&0xFFF)<<endl;
#endif
                    int qdcch = (gg[i]>>16)&0x1F;
                    int qdcdata =(gg[i]&0xFFF);
                    double ecal=qdcdata*ch_ecal1[qdcch]+ch_ecal0[qdcch];                    
                    if (qdcdata>ch_thr[qdcch]&&qdcdata<OVER_FLOW-10) {
                        h1tmp->Fill(qdcch);
                    }
                    hadc[qdcch]->Fill(ecal);
                    h2->Fill(qdcch,ecal);
                    nadcdatum++;
                }else if (((gg[i]&0x7000000)>>24)==0x4){//end of qdc block met
                    int qdcevtcounter = gg[i]&0xFFFFFF;
                    lastcounter = qdcevtcounter;
#ifdef DEBUG
                    cout<<"END of Event No. = "<<qdcevtcounter<<endl;
#endif
                }else if (((gg[i]&0x7000000)>>24)==0x6){
                    flag_not_valid_dantum=true;
#ifdef DEBUG
                    //cout<<"empty buffer read"<<endl;
#endif
                }
                nwordqdc++;
            }
            if ((gg[i]&0xF8000000)>>27!=0x13&&flag_end_v792counter){
                now_daq_ncounter=gg[i];
#ifdef DEBUG
                cout<<"END of Block, DAQ counter = "<<now_daq_ncounter<<endl;
#endif
            }
        }
#ifdef DEBUG
        if (nadcdatum<5) cout<<"Invalid dantum!"<<nadcdatum<<endl;
#endif
        //cout<<"EVENT END "<<nwordqdc<<endl;
        delete adcp;
    }
    return 0;
}
