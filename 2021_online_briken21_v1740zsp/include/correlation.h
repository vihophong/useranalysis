#ifndef CORRELATION_h
#define CORRELATION_h 1

#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TString.h>
#include <TSpline.h>
#include <map>
#include <vector>
#include <libDataStruct.h>
#include <TTree.h>

#define MAX_N_HISTO 20

class correlation
{
public:
  correlation(int b1,int b2,Long64_t tw_low, Long64_t tw_high);
  correlation(Long64_t tw_low, Long64_t tw_high);
  virtual ~correlation();
  void SetBMap(int b1, int b2){fbmap1 = b1;fbmap2 = b2;}
  void SetTimeWindow(Long64_t tw_low, Long64_t tw_high){ftw_low = tw_low;ftw_high = tw_high;}
  void SetMaxMapLength(unsigned long maxmaplength){fmax_map_length = maxmaplength;}
  void fillMap(bool flagend, NIGIRI* data);
  void SetMultipleCorrelation(){is_single_corr=false;}
  void SetHisto1D(TH1F* h1,Int_t id){fhcorr1d[id]=h1;}
  void SetHisto2D(TH2F* h2,Int_t id){fhcorr2d[id]=h2;}
  void SetTree(TTree* tree){ftreecorr=tree;}
private:
  void reconstruct();
  std::multimap <Long64_t, NIGIRI*> map1;
  std::multimap <Long64_t, NIGIRI*>::iterator map1_it;
  std::multimap <Long64_t, NIGIRI*> map2;
  std::multimap <Long64_t, NIGIRI*>::iterator map2_it;

  int map2_mincorr_distance_prev;

  bool is_single_corr;
  unsigned long fmax_map_length;
  int fbmap1,fbmap2;
  int ftw_low, ftw_high;

  ULong64_t fncorr;
  bool is_self_corr;

  TH1F* fhcorr1d[MAX_N_HISTO];
  TH2F* fhcorr2d[MAX_N_HISTO];
  TTree* ftreecorr;
};
#endif

