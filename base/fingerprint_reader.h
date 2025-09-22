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

// Retrieves and parses the fingerprint configuration from the command line
// Uses the --fingerprint switch to specify the path to the configuration file
// Returns empty string if no fingerprint file is specified
std::string GetFingerprintFromCommandLine();

// Reads and parses a fingerprint configuration file
// Returns nullptr if the file cannot be read or parsed
std::unique_ptr<FingerprintConfig> GetFingerprintFromFile(
    const std::string& file_path);

COMPONENT_EXPORT(FINGERPRINTING_CORE) bool InitializeFingerprintForTesting();

}  // namespace fingerprinting::core


#endif  // BASE_FINGERPRINT_READER_H_
