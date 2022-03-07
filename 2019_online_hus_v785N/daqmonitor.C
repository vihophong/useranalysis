#include "stdio.h"
#include "string.h"
void daqmonitor() {
  gROOT->ProcessLine(".L libmyanalysis.so");
  gROOT->ProcessLine("gSystem->Load(\"libpmonitor\")");
  gROOT->ProcessLine("rcdaqopen()");
  gROOT->ProcessLine("pstart()");
}
