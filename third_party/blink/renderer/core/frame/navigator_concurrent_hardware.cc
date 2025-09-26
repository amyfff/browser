// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/frame/navigator_concurrent_hardware.h"

#include "base/system/sys_info.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "chrome/browser/fingerprint_manager.h"

namespace blink {

namespace {

// TODO(435582603): Hard-coding this to a common value is a reasonable start,
// but it likely makes sense to vary the hard-coded number by platform and
// form-factor in order to maintain plausibility over time.
constexpr unsigned kReducedHardwareConcurrencyValue = 8u;

}  // namespace

unsigned NavigatorConcurrentHardware::hardwareConcurrency() const {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch("fingerprint")) {
    std::string fingerprint_path =
        base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII("fingerprint");

    LOG(INFO) << "Fingerprint path: " << fingerprint_path;
    auto* fm = fingerprinting::FingerprintManager::GetInstance();
    if (fm->Initialize(fingerprint_path)) {
      // Use the CPU cores value from fingerprint manager
      return static_cast<unsigned>(fm->GetCpuCores());
    } else {
      LOG(ERROR) << "Failed to initialize fingerprint manager";
    }
  } else {
    LOG(INFO) << "Fingerprint switch not present, skipping fingerprint initialization.";
  }
  
  if (RuntimeEnabledFeatures::ReduceHardwareConcurrencyEnabled()) {
    return kReducedHardwareConcurrencyValue;
  }
  return static_cast<unsigned>(base::SysInfo::NumberOfProcessors());
}

}  // namespace blink
