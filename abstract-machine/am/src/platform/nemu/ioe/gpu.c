#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t reg = inl(VGACTL_ADDR);         
  uint32_t w = reg >> 16;                  
  uint32_t h = reg & 0xffff;              
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true,
    .has_accel = false,
    .width = w,
    .height = h,
    .vmemsz = w * h * sizeof(uint32_t)
  };
}
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;

  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *pixels = ctl->pixels;
  for (int row = 0; row < ctl->h; row++) {
    uint32_t *dst = fb + (ctl->y + row) * screen_w + ctl->x;
    uint32_t *src = pixels + row * ctl->w;
    for (int col = 0; col < ctl->w; col++) {
      dst[col] = src[col];
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}
void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
