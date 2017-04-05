#include "SysTickImpl.h"

SysTickImpl::SysTickImpl(void)
{
  CONTROL = (int*) 0xE000E010;
  LOAD =  (int*) 0xE000E014;
  VAL = (int*) 0xE000E018;
  CALIB = (int*) 0xE000E01C;
}

int SysTickImpl::GetControl() {
  return *CONTROL;
}

int SysTickImpl::GetLoad() {
  return *LOAD;
}

void SysTickImpl::SetLoad(int val) {
  *LOAD = val;
}

int SysTickImpl::GetVal() {
  return *VAL;
}

int SysTickImpl::GetCalib() {
  return *CALIB;
}

