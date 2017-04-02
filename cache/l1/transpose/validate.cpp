#include <iostream>
#include "perf_test.h"

#define fileName "validate.co"
#define kernelName "hello_world"

#define X 256
#define Y 16
#define LEN X*Y
#define WI 256
#define WG 1
#define SIZE LEN<<2
#define ITER 0x200000


int main() {
  float *Ah, *Bh;
  hipDeviceptr_t Ad, Bd;
  Ah = new float[LEN];
  Bh = new float[LEN];

  for(unsigned i=0;i<LEN;i++) {
    Ah[i] = 11.0f;
    Bh[i] = 10.0f;
  }

  CHECK(hipInit(0));
  hipDevice_t device;
  hipCtx_t context;
  CHECK(hipDeviceGet(&device, 0));
  CHECK(hipCtxCreate(&context, 0, device));
  hipModule_t Module;
  hipFunction_t Function;

  CHECK(hipMalloc((void**)&Ad, SIZE));
  CHECK(hipMalloc((void**)&Bd, SIZE));

  CHECK(hipMemcpyHtoD(Ad, Ah, SIZE));
  CHECK(hipMemcpyHtoD(Bd, Bh, SIZE));

  CHECK(hipModuleLoad(&Module, fileName));
  CHECK(hipModuleGetFunction(&Function, Module, kernelName));

  struct {
    void *Ad;
    void *Bd;
  } args;

  args.Ad = Ad;
  args.Bd = Bd;

  size_t size = sizeof(args);

  void *config[] = {
    HIP_LAUNCH_PARAM_BUFFER_POINTER, &args,
    HIP_LAUNCH_PARAM_BUFFER_SIZE, &size,
    HIP_LAUNCH_PARAM_END
  };

  CHECK(hipModuleLaunchKernel(Function, WG,1,1, WI,1,1, 0, 0, NULL, (void**)&config));
  CHECK(hipDeviceSynchronize());
  hipMemcpyDtoH(Ah, Ad, SIZE);
  int *iPtr = (int*)Ah;
  for(int i=0;i<X;i++){
    std::cout<<Ah[i]<<" "<<iPtr[i]<<std::endl;
  }

}
