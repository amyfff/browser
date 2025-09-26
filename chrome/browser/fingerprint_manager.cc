#include "fingerprint_manager.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/logging.h"

namespace fingerprinting {

static base::LazyInstance<FingerprintManager>::Leaky g_fingerprint_manager = LAZY_INSTANCE_INITIALIZER;

FingerprintManager* FingerprintManager::GetInstance() {
  return g_fingerprint_manager.Pointer();
}

bool FingerprintManager::Initialize(const std::string& path) {
  LOG(INFO) << "Fingerprint Manager Initialized";
  LOG(INFO) << "Loading fingerprint from: " << path;

  absl::optional<base::Value> parsed_json = LoadJsonFromFile(path);
  if (!parsed_json) {
    LOG(ERROR) << "Failed to load fingerprint from file: " << path;
    return false;
  }

  const base::Value::Dict& root = parsed_json->GetDict();
  if (!ParseBasicInfo(root)) {
    LOG(ERROR) << "Failed to parse basic fingerprint information";
    return false;
  }

  return true;
}

bool FingerprintManager::LoadFromString(const std::string& json_string) {
  if (json_string.empty()) {
    LOG(ERROR) << "Empty JSON string provided";
    return false;
  }

  auto parsed_json = base::JSONReader::ReadAndReturnValueWithError(json_string);
  if (!parsed_json.has_value() || !parsed_json->is_dict()) {
    LOG(ERROR) << "Invalid JSON or not an object";
    return false;
  }

  const base::Value::Dict& root = parsed_json->GetDict();
  return ParseBasicInfo(root);
}

absl::optional<base::Value> FingerprintManager::LoadJsonFromFile(
    const std::string& file_path) const {
  if (file_path.empty()) {
    LOG(ERROR) << "Empty fingerprint file path provided";
    return absl::nullopt;
  }

  std::string file_content;
  if (!base::ReadFileToString(base::FilePath::FromUTF8Unsafe(file_path), 
                             &file_content)) {
    LOG(ERROR) << "Failed reading fingerprint file: " << file_path;
    return absl::nullopt;
  }

  auto parsed_json = base::JSONReader::ReadAndReturnValueWithError(file_content);
  if (!parsed_json.has_value() || !parsed_json->is_dict()) {
    LOG(ERROR) << "Invalid JSON or not an object in file: " << file_path;
    return absl::nullopt;
  }

  return std::move(*parsed_json);
}

bool FingerprintManager::ParseBasicInfo(const base::Value::Dict& root) {
  const std::string* mode = root.FindString("fingerprint_mode");
  const std::string* id = root.FindString("fingerprint_id");
  const std::string* creation_time = root.FindString("creation_time");
  const base::Value::Dict* data_dict = root.FindDict("data");

  if (!mode || !id || !creation_time) {
    LOG(ERROR) << "Missing required basic fields (mode, id, or creation_time)";
    return false;
  }

  config_ = std::make_unique<BasicInfo>();
  config_->mode = *mode;
  config_->id = *id;
  config_->creation_time = *creation_time;

  if (data_dict) {
    config_->data = data_dict->Clone();
  }

  LOG(INFO) << "Fingerprint loaded successfully:"
            << "\n  Mode: " << config_->mode
            << "\n  ID: " << config_->id
            << "\n  Creation Time: " << config_->creation_time
            << "\n  Data Entries: " << config_->data;
  return true;
}

bool FingerprintManager::IsEnabled(const std::string& key) const {
  if (!config_) {
    LOG(ERROR) << "FingerprintManager not initialized!";
    return false;
  }

  absl::optional<bool> flag = config_->data.FindBool(key);
  if (!flag.has_value()) {
    LOG(WARNING) << "Flag '" << key << "' not found in fingerprint data";
    return false;
  }

  LOG(INFO) << "Flag '" << key << "' is set to " << (*flag ? "true" : "false");
  return *flag;
}

int FingerprintManager::GetCpuCores() const {
  if (!config_) {
    LOG(ERROR) << "FingerprintManager not initialized!";
    return 8;  // Default value if not initialized
  }

  const base::Value::Dict* hardware = config_->data.FindDict("hardware");
  if (!hardware) {
    LOG(WARNING) << "Hardware section not found in fingerprint data";
    return 8;
  }

  const base::Value::Dict* cpu = hardware->FindDict("cpu");
  if (!cpu) {
    LOG(WARNING) << "CPU section not found in hardware data";
    return 8;
  }

  bool mode = cpu->FindBool("mode").value_or(false);
  if (!mode) {
    return 8;  // Return default value when mode is false
  }

  return cpu->FindInt("cores").value_or(8);  // Return cores value or default 8 if not found
}

int FingerprintManager::GetDeviceMemory() const {
  if (!config_) {
    LOG(ERROR) << "FingerprintManager not initialized!";
    return 8;  // Default value if not initialized
  }

  const base::Value::Dict* hardware = config_->data.FindDict("hardware");
  if (!hardware) {
    LOG(WARNING) << "Hardware section not found in fingerprint data";
    return 8;
  }

  const base::Value::Dict* memory = hardware->FindDict("memory");
  if (!memory) {
    LOG(WARNING) << "Memory section not found in hardware data";
    return 8;
  }

  bool mode = memory->FindBool("mode").value_or(false);
  if (!mode) {
    return 8;  // Return default value when mode is false
  }

  return memory->FindInt("device_memory").value_or(8);  // Return memory value or default 8 if not found
}

}  // namespace fingerprinting