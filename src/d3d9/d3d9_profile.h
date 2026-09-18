#pragma once
// Lightweight main-thread profiler for the D3D9 entry points (diagnostic builds only).
// Accumulates wall time spent inside each category and logs a per-frame summary every N Presents.
#include <chrono>
#include <cstdint>
#include "../util/log/log.h"
#include "../util/util_string.h"

namespace dxvk::prof {

  enum Cat : uint32_t { Draw, DrawUP, SetTexture, SetState, Lock, Unlock, Upload, Present, Clear, Create, Other, PFlush, PAcquire, PRecord, PSubmit, PSyncLat, Count };
  static const char* const kNames[Count] = { "draw", "drawUP", "setTexture", "setState", "lock", "unlock", "upload", "present", "clear", "create", "other", "p.flush", "p.acquire", "p.record", "p.submit", "p.syncLatency" };

  struct Acc {
    uint64_t ns[Count]    = { };
    uint64_t calls[Count] = { };
    uint32_t frames       = 0;
    uint64_t frameNs      = 0;
    std::chrono::steady_clock::time_point lastPresent { };
  };

  inline Acc& acc() { static Acc a; return a; }

  inline uint64_t nowNs() {
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
  }

  struct Scope {
    Cat c; uint64_t t0;
    explicit Scope(Cat cat) : c(cat), t0(nowNs()) { }
    ~Scope() { Acc& a = acc(); a.ns[c] += nowNs() - t0; a.calls[c]++; }
  };

  inline void frameEnd() {
    Acc& a = acc();
    auto now = std::chrono::steady_clock::now();
    if (a.lastPresent.time_since_epoch().count() != 0)
      a.frameNs += uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(now - a.lastPresent).count());
    a.lastPresent = now;
    if (++a.frames < 300) return;
    double f = double(a.frames);
    double inside = 0; for (uint32_t i = 0; i < Count; i++) inside += a.ns[i];
    std::string line = str::format("[prof] frames=", a.frames,
      " ms/frame: wall=", a.frameNs / 1e6 / f, " insideD3D9=", inside / 1e6 / f);
    for (uint32_t i = 0; i < Count; i++)
      line += str::format(" ", kNames[i], "=", a.ns[i] / 1e6 / f, "ms/", a.calls[i] / f, "x");
    Logger::info(line);
    a = Acc();
    a.lastPresent = now;
  }
}
