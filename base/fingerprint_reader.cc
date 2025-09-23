#include "base/fingerprint_reader.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace fingerprinting::core {

bool InitializeFingerprintForTesting(const std::string& file_path) {
    LOG(INFO) << "Attempting to load fingerprint from: " << file_path;
    
    auto fingerprint_config = GetFingerprintFromFile(file_path);
    if (!fingerprint_config) {
        LOG(ERROR) << "Failed to load fingerprint configuration from file: " << file_path;
        return false;
    }
    
    LOG(INFO) << "Successfully loaded fingerprint configuration";
    LOG(INFO) << "Mode: " << fingerprint_config->mode;
    LOG(INFO) << "ID: " << fingerprint_config->id;
    LOG(INFO) << "Creation Time: " << fingerprint_config->creation_time;
    LOG(INFO) << "Data: " << fingerprint_config->data;

    return true;
}

std::unique_ptr<FingerprintConfig> GetFingerprintFromFile(
    const std::string& file_path) {
    if (file_path.empty()) {
        LOG(ERROR) << "Empty fingerprint file path provided";
        return nullptr;
    }

    base::FilePath file_path_obj = base::FilePath::FromUTF8Unsafe(file_path);

    std::string file_content;
    if (!base::ReadFileToString(file_path_obj, &file_content)) {
        LOG(ERROR) << "Failed reading: " << file_path;
        return nullptr;
    }
    
    if (file_content.empty()) {
        LOG(ERROR) << "Fingerprint file is empty: " << file_path;
        return nullptr;
    }

    absl::optional<base::Value> parsed_json = base::JSONReader::Read(file_content);
    if (!parsed_json || !parsed_json->is_dict()) {
        LOG(ERROR) << "Invalid JSON in fingerprint file or not an object";
        return nullptr;
    }

    const base::Value::Dict& dict = parsed_json->GetDict();

    const std::string* mode = dict.FindString("fingerprint_mode");
    if (!mode) {
        LOG(ERROR) << "Missing required field: fingerprint_mode";
        return nullptr;
    }

    const std::string* id = dict.FindString("fingerprint_id");
    if (!id) {
        LOG(ERROR) << "Missing required field: fingerprint_id";
        return nullptr;
    }

    const std::string* creation_time = dict.FindString("creation_time");
    if (!creation_time) {
        LOG(ERROR) << "Missing required field: creation_time";
        return nullptr;
    }

    const base::Value::Dict* data = dict.FindDict("data");
    if (!data) {
        LOG(ERROR) << "Missing required field: data";
        return nullptr;
    }

    LOG(INFO) << "Fingerprint loaded from file:"
              << "\n  Mode: " << *mode
              << "\n  ID: " << *id
              << "\n  Creation Time: " << *creation_time;

    auto config = std::make_unique<FingerprintConfig>();
    config->mode = *mode;
    config->id = *id;
    config->creation_time = *creation_time;
    config->data = data->Clone();

    return config;
}

}  // namespace fingerprinting::core