class SysTickImpl {
  private:
  // Control and status register addr
  int* CONTROL;

  // Reload value register addr
  int* LOAD;

  // Current value register addr
  int* VAL;

  // Calibration value register addr
  int* CALIB;

  public:
  SysTickImpl();

  int GetControl();
  int GetLoad();
  void SetLoad(int val);
  int GetVal();
  int GetCalib();
};

