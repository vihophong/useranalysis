#ifndef __MYANALYSIS_H__
#define __MYANALYSIS_H__

#include <Event/Event.h>
#include <iostream>
#include <vector>
#include <correlation.h>

int process_event (Event *e); //++CINT 
int ppclose();

typedef struct{
    int size;
    int board_id;
    bool board_fail_flag;
    int pattern;
    int dual_ch_mask;
    int counter;
    int timetag;
    int print(){
        std::cout<<"board aggregate:"<<std::endl;
        std::cout<<"boardaggr_t->size = "<<size<<std::endl;
        std::cout<<"boardaggr_t->board_id = "<<board_id<<std::endl;
        std::cout<<"boardaggr_t->board_fail_flag = "<<board_fail_flag<<std::endl;
        std::cout<<"boardaggr_t->pattern = "<<pattern<<std::endl;
        std::cout<<"boardaggr_t->dual_ch_mask = "<<dual_ch_mask<<std::endl;
        std::cout<<"boardaggr_t->counter = "<<counter<<std::endl;
        std::cout<<"boardaggr_t->timetag = "<<timetag<<std::endl;
        std::cout<<"--------------"<<std::endl;
        return 0;
    }
}boardaggr_t;

typedef struct{
    int groupid;
    bool formatinfo;
    int size;
    bool dual_trace_flag;
    bool energy_enable_flag;
    bool trigger_time_stamp_enable_flag;
    bool extra2_enable_flag;
    bool waveformsample_enable_flag;
    short extra_option_enable_flag;
    short analogprobe1_selection;
    short analogprobe2_selection;
    short digitalprobe_selection;
    int n_samples;
    int print(){
        std::cout<<"channel aggregate:"<<std::endl;
        std::cout<<"channelaggr_t->id = "<<groupid<<std::endl;
        std::cout<<"channelaggr_t->formatinfo = "<<formatinfo<<std::endl;
        std::cout<<"channelaggr_t->size = "<<size<<std::endl;
        std::cout<<"channelaggr_t->dual_trace_flag = "<<dual_trace_flag<<std::endl;
        std::cout<<"channelaggr_t->energy_enable_flag = "<<energy_enable_flag<<std::endl;
        std::cout<<"channelaggr_t->trigger_time_stamp_enable_flag = "<<trigger_time_stamp_enable_flag<<std::endl;
        std::cout<<"channelaggr_t->extra2_enable_flag = "<<extra2_enable_flag<<std::endl;
        std::cout<<"channelaggr_t->waveformsample_enable_flag = "<<waveformsample_enable_flag<<std::endl;
        std::cout<<"channelaggr_t->extra_option_enable_flag = "<<extra_option_enable_flag<<std::endl;
        std::cout<<"channelaggr_t->analogprobe1_selection = "<<analogprobe1_selection<<std::endl;
        std::cout<<"channelaggr_t->analogprobe2_selection = "<<analogprobe2_selection<<std::endl;
        std::cout<<"channelaggr_t->digitalprobe_selection = "<<digitalprobe_selection<<std::endl;
        std::cout<<"channelaggr_t->n_samples = "<<n_samples<<std::endl;
        std::cout<<"--------------"<<std::endl;
        return 0;
    }
}channelaggr_t;
typedef struct{
    int ch;
    int trigger_time_tag;
    int n_samples;
    int extras2;
    int extras;
    bool pileup_rollover_flag;
    int energy;
    std::vector <UShort_t> ap1_sample;
    std::vector <UShort_t> ap2_sample;
    std::vector <UShort_t> dp_sample;
    std::vector <UShort_t> trg_sample;

    int print(){
        std::cout<<"channel data:"<<std::endl;
        std::cout<<"channel_t->ch = "<<ch<<std::endl;
        std::cout<<"channel_t->trigger_time_tag = "<<trigger_time_tag<<std::endl;
        std::cout<<"channel_t->n_samples = "<<n_samples<<std::endl;
        std::cout<<"channel_t->extras2 = "<<extras2<<std::endl;
        std::cout<<"channel_t->extras = "<<extras<<std::endl;
        std::cout<<"channel_t->pileup_rollover_flag = "<<pileup_rollover_flag<<std::endl;
        std::cout<<"channel_t->energy = "<<energy<<std::endl;
        std::cout<<"channel_t->ap1_sample_size = "<<ap1_sample.size()<<std::endl;
        std::cout<<"channel_t->ap2_sample_size = "<<ap2_sample.size()<<std::endl;
        std::cout<<"channel_t->dp_sample_size = "<<dp_sample.size()<<std::endl;
        std::cout<<"channel_t->trg_sample_size = "<<trg_sample.size()<<std::endl;
        std::cout<<"--------------"<<std::endl;
        return 0;
    }
}channel_t;
#endif /* __MYANALYSIS_H__ */
