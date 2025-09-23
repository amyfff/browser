#ifndef BASE_FINGERPRINT_READER_H_
#define BASE_FINGERPRINT_READER_H_

#include <memory>
#include <string>
#include "base/component_export.h"
#include "base/values.h"

namespace fingerprinting::core {

// Structure representing a fingerprint configuration
struct COMPONENT_EXPORT(FINGERPRINTING_CORE) FingerprintConfig {
  std::string mode;                    // "random" or other modes
  std::string id;                      // UUID string
  std::string creation_time;           // Timestamp string
  base::Value::Dict data;              // Additional fingerprint data
};

// Initializes fingerprint configuration from specified file path
// Returns true if successfully initialized, false otherwise
bool COMPONENT_EXPORT(FINGERPRINTING_CORE) InitializeFingerprintForTesting(const std::string& file_path);

// Reads and parses a fingerprint configuration file
// Returns nullptr if the file cannot be read or parsed
std::unique_ptr<FingerprintConfig> GetFingerprintFromFile(
    const std::string& file_path);

}  // namespace fingerprinting::core

#endif  // BASE_FINGERPRINT_READER_H_