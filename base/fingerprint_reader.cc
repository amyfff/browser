#include "base/fingerprint_reader.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace fingerprinting::core {

bool InitializeFingerprintForTesting() {
    std::string fingerprint_path = GetFingerprintFromCommandLine();
    if (fingerprint_path.empty()) {
        LOG(INFO) << "No fingerprint configuration file specified.";
        return false;
    }
    
    LOG(INFO) << "Attempting to load fingerprint from: " << fingerprint_path;
    
    auto fingerprint_config = GetFingerprintFromFile(fingerprint_path);
    if (!fingerprint_config) {
        LOG(ERROR) << "Failed to load fingerprint configuration from file: " << fingerprint_path;
        return false;
    }
    
    LOG(INFO) << "Successfully loaded fingerprint configuration";
    LOG(INFO) << "Mode: " << fingerprint_config->mode;
    LOG(INFO) << "ID: " << fingerprint_config->id;
    LOG(INFO) << "Creation Time: " << fingerprint_config->creation_time;
    LOG(INFO) << "Data: " << fingerprint_config->data;

    return true;
}

std::string GetFingerprintFromCommandLine() {
    const base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
    
    // Check if the --fingerprint switch is present
    if (!command_line->HasSwitch("fingerprint")) {
        LOG(INFO) << "No fingerprint configuration file specified.";
        return std::string();
    }

    // Get the file path from the switch value
    std::string file_path = command_line->GetSwitchValueASCII("fingerprint");
    if (file_path.empty()) {
        LOG(ERROR) << "Empty fingerprint configuration file path provided.";
        return std::string();
    }

    return file_path;
}

std::unique_ptr<FingerprintConfig> GetFingerprintFromFile(
    const std::string& file_path) {
    // Check for empty path
    if (file_path.empty()) {
        LOG(ERROR) << "Empty fingerprint file path provided";
        return nullptr;
    }

    // Create FilePath object from UTF8 string
    base::FilePath file_path_obj = base::FilePath::FromUTF8Unsafe(file_path);

    // Check if file exists and read its content
    std::string file_content;
    if (!base::ReadFileToString(file_path_obj, &file_content)) {
        LOG(ERROR) << "Failed reading: " << file_path;
        return nullptr;
    }
    if (file_content.empty()) {
        LOG(ERROR) << "Fingerprint file is empty: " << file_path;
        return nullptr;
    }
    // Parse JSON with base::JSONReader
    absl::optional<base::Value> parsed_json = base::JSONReader::Read(file_content);
    if (!parsed_json || !parsed_json->is_dict()) {
        LOG(ERROR) << "Invalid JSON in fingerprint file or not an object";
        return nullptr;
    }

    const base::Value::Dict& dict = parsed_json->GetDict();

    // Validate required fields with specific error messages
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

    // Create and return the config
    auto config = std::make_unique<FingerprintConfig>();
    config->mode = *mode;
    config->id = *id;
    config->creation_time = *creation_time;
    config->data = data->Clone();

    return config;
}

}  // namespace fingerprinting::core