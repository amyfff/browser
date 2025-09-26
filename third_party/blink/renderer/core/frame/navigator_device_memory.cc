// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/frame/navigator_device_memory.h"

#include "third_party/blink/public/common/device_memory/approximated_device_memory.h"
#include "third_party/blink/public/common/privacy_budget/identifiability_metric_builder.h"
#include "base/command_line.h"
#include "chrome/browser/fingerprint_manager.h"
#include "base/logging.h"
#include "third_party/blink/public/common/privacy_budget/identifiability_metrics.h"
#include "third_party/blink/public/mojom/use_counter/metrics/web_feature.mojom-shared.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"

namespace blink {

namespace {

// TODO(435582603): Hard-coding this to a common value is a reasonable start,
// but it likely makes sense to vary the hard-coded number by platform and
// form-factor in order to maintain plausibility over time.
constexpr float kReducedDeviceMemoryValue = 8.0;

}  // namespace

float NavigatorDeviceMemory::deviceMemory() const {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch("fingerprint")) {
    std::string fingerprint_path =
        base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII("fingerprint");

    LOG(INFO) << "Fingerprint path for device memory: " << fingerprint_path;
    auto* fm = fingerprinting::FingerprintManager::GetInstance();
    if (fm->Initialize(fingerprint_path)) {
      // Use the device memory value from fingerprint manager
      return static_cast<float>(fm->GetDeviceMemory());
    } else {
      LOG(ERROR) << "Failed to initialize fingerprint manager for device memory";
    }
  } else {
    LOG(INFO) << "Fingerprint switch not present, using default device memory logic";
  }

  if (RuntimeEnabledFeatures::ReduceDeviceMemoryEnabled()) {
    return kReducedDeviceMemoryValue;
  }
  return ApproximatedDeviceMemory::GetApproximatedDeviceMemory();
}

}  // namespace blink
