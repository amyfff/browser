#ifndef BASE_FINGERPRINT_MANAGER_H_
#define BASE_FINGERPRINT_MANAGER_H_

#include <memory>
#include <string>

#include "base/lazy_instance.h"
#include "base/values.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace fingerprinting {

struct BasicInfo {
  std::string mode;
  std::string id;
  std::string creation_time;
  base::Value::Dict data;
};

class FingerprintManager {
public:
  static FingerprintManager* GetInstance();

  bool Initialize(const std::string& path);
  bool LoadFromString(const std::string& json_string);
  bool IsEnabled(const std::string& key) const;
  const BasicInfo* GetConfig() const { return config_.get(); }
  int GetCpuCores() const;
  int GetDeviceMemory() const;
  
private:
  FingerprintManager() = default;
  ~FingerprintManager() = default;

  friend struct base::internal::LeakyLazyInstanceTraits<FingerprintManager>;
  friend struct base::LazyInstanceTraitsBase<FingerprintManager>;
  
  absl::optional<base::Value> LoadJsonFromFile(const std::string& file_path) const;
  bool ParseBasicInfo(const base::Value::Dict& root);
  
  std::unique_ptr<BasicInfo> config_;
};

}  // namespace fingerprinting

#endif  // BASE_FINGERPRINT_MANAGER_H_