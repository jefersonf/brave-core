/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <utility>

#include "base/environment.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "base/values.h"
#include "brave/components/brave_wallet/browser/pref_names.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "brave/components/brave_wallet/common/features.h"
#include "brave/components/brave_wallet/common/value_conversion_utils.h"
#include "brave/third_party/ethash/src/include/ethash/keccak.h"
#include "brave/vendor/bip39wally-core-native/include/wally_bip39.h"
#include "components/prefs/pref_service.h"
#include "crypto/random.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "url/gurl.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace brave_wallet {

namespace {
std::string GenerateMnemonicInternal(uint8_t* entropy, size_t size) {
  char* words = nullptr;
  std::string result;
  if (bip39_mnemonic_from_bytes(nullptr, entropy, size, &words) != WALLY_OK) {
    LOG(ERROR) << __func__ << ": bip39_mnemonic_from_bytes failed";
    return result;
  }
  result = words;
  wally_free_string(words);
  return result;
}

bool IsValidEntropySize(size_t entropy_size) {
  // entropy size should be 128, 160, 192, 224, 256 bits
  if (entropy_size < 16 || entropy_size > 32 || entropy_size % 4 != 0) {
    LOG(ERROR) << __func__ << ": Entropy should be 16, 20, 24, 28, 32 bytes";
    return false;
  }
  return true;
}

std::string GetInfuraProjectID() {
  std::string project_id(BRAVE_INFURA_PROJECT_ID);
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  if (env->HasVar("BRAVE_INFURA_PROJECT_ID")) {
    env->GetVar("BRAVE_INFURA_PROJECT_ID", &project_id);
  }
  return project_id;
}

bool GetUseStagingInfuraEndpoint() {
  std::string project_id(BRAVE_INFURA_PROJECT_ID);
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  return env->HasVar("BRAVE_INFURA_STAGING");
}

const char kGanacheLocalhostURL[] = "http://localhost:7545/";

// Precompiled networks available in native wallet.
const brave_wallet::mojom::EthereumChain kKnownNetworks[] = {
    {brave_wallet::mojom::kMainnetChainId,
     "Ethereum Mainnet",
     {"https://etherscan.io"},
     {},
     {},
     "ETH",
     "Ethereum",
     18,
     true},
    {brave_wallet::mojom::kRinkebyChainId,
     "Rinkeby Test Network",
     {"https://rinkeby.etherscan.io"},
     {},
     {},
     "ETH",
     "Ethereum",
     18,
     true},
    {brave_wallet::mojom::kRopstenChainId,
     "Ropsten Test Network",
     {"https://ropsten.etherscan.io"},
     {},
     {},
     "ETH",
     "Ethereum",
     18,
     true},
    {brave_wallet::mojom::kGoerliChainId,
     "Goerli Test Network",
     {"https://goerli.etherscan.io"},
     {},
     {},
     "ETH",
     "Ethereum",
     18,
     true},
    {brave_wallet::mojom::kKovanChainId,
     "Kovan Test Network",
     {"https://kovan.etherscan.io"},
     {},
     {},
     "ETH",
     "Ethereum",
     18,
     true},
    {brave_wallet::mojom::kLocalhostChainId,
     "Localhost",
     {kGanacheLocalhostURL},
     {},
     {kGanacheLocalhostURL},
     "ETH",
     "Ethereum",
     18,
     false}};

const base::flat_map<std::string, std::string> kInfuraSubdomains = {
    {brave_wallet::mojom::kMainnetChainId, "mainnet"},
    {brave_wallet::mojom::kRinkebyChainId, "rinkeby"},
    {brave_wallet::mojom::kRopstenChainId, "ropsten"},
    {brave_wallet::mojom::kGoerliChainId, "goerli"},
    {brave_wallet::mojom::kKovanChainId, "kovan"}};

const base::flat_map<std::string, std::string>
    kUnstoppableDomainsProxyReaderContractAddressMap = {
        {brave_wallet::mojom::kMainnetChainId,
         "0xa6E7cEf2EDDEA66352Fd68E5915b60BDbb7309f5"},
        {brave_wallet::mojom::kRinkebyChainId,
         "0x3A2e74CF832cbA3d77E72708d55370119E4323a6"}};

const base::flat_map<std::string, std::string> kEnsRegistryContractAddressMap =
    {{brave_wallet::mojom::kMainnetChainId,
      "0x00000000000C2E074eC69A0dFb2997BA6C7d2e1e"},
     {brave_wallet::mojom::kRopstenChainId,
      "0x00000000000C2E074eC69A0dFb2997BA6C7d2e1e"},
     {brave_wallet::mojom::kRinkebyChainId,
      "0x00000000000C2E074eC69A0dFb2997BA6C7d2e1e"},
     {brave_wallet::mojom::kGoerliChainId,
      "0x00000000000C2E074eC69A0dFb2997BA6C7d2e1e"}};

std::string GetInfuraURLForKnownChainId(const std::string& chain_id) {
  auto subdomain = brave_wallet::GetInfuraSubdomainForKnownChainId(chain_id);
  if (subdomain.empty())
    return std::string();
  return base::StringPrintf(
      GetUseStagingInfuraEndpoint()
          ? "https://%s-staging-infura.bravesoftware.com/%s"
          : "https://%s-infura.brave.com/%s",
      subdomain.c_str(), GetInfuraProjectID().c_str());
}

GURL GetCustomChainURL(PrefService* prefs, const std::string& chain_id) {
  std::vector<brave_wallet::mojom::EthereumChainPtr> custom_chains;
  brave_wallet::GetAllCustomChains(prefs, &custom_chains);
  for (const auto& it : custom_chains) {
    if (it->chain_id != chain_id)
      continue;
    if (it->rpc_urls.empty())
      return GURL();
    return GURL(it->rpc_urls.front());
  }
  return GURL();
}

}  // namespace

mojom::EthereumChainPtr GetKnownChain(PrefService* prefs,
                                      const std::string& chain_id) {
  for (const auto& network : kKnownNetworks) {
    if (network.chain_id != chain_id)
      continue;

    auto result = network.Clone();
    if (chain_id == brave_wallet::mojom::kLocalhostChainId)
      result->is_eip1559 = prefs->GetBoolean(kSupportEip1559OnLocalhostChain);
    if (result->rpc_urls.empty())
      result->rpc_urls.push_back(GetInfuraURLForKnownChainId(chain_id));
    return result;
  }
  return nullptr;
}

std::string GetInfuraSubdomainForKnownChainId(const std::string& chain_id) {
  if (kInfuraSubdomains.contains(chain_id))
    return kInfuraSubdomains.at(chain_id);
  return std::string();
}

void GetAllCustomChains(PrefService* prefs,
                        std::vector<mojom::EthereumChainPtr>* result) {
  const base::Value* custom_networks_list =
      prefs->GetList(kBraveWalletCustomNetworks);
  if (!custom_networks_list)
    return;
  for (const auto& it : custom_networks_list->GetList()) {
    absl::optional<mojom::EthereumChain> chain =
        brave_wallet::ValueToEthereumChain(it);
    if (chain)
      result->push_back(chain->Clone());
  }
}

bool IsNativeWalletEnabled() {
  return base::FeatureList::IsEnabled(
      brave_wallet::features::kNativeBraveWalletFeature);
}

const std::vector<brave_wallet::mojom::EthereumChain>
GetAllKnownNetworksForTesting() {
  std::vector<brave_wallet::mojom::EthereumChain> result;
  result.assign(kKnownNetworks, std::end(kKnownNetworks));
  return result;
}

std::string ToHex(const std::string& data) {
  if (data.empty()) {
    return "0x0";
  }
  return "0x" + base::ToLowerASCII(base::HexEncode(data.data(), data.size()));
}

std::string ToHex(const std::vector<uint8_t>& data) {
  if (data.empty())
    return "0x0";
  return "0x" + base::ToLowerASCII(base::HexEncode(data));
}

std::string KeccakHash(const std::string& input, bool to_hex) {
  std::vector<uint8_t> bytes(input.begin(), input.end());
  std::vector<uint8_t> result = KeccakHash(bytes);
  std::string result_str(result.begin(), result.end());
  return to_hex ? ToHex(result_str) : result_str;
}

std::vector<uint8_t> KeccakHash(const std::vector<uint8_t>& input) {
  auto hash = ethash_keccak256(input.data(), input.size());
  return std::vector<uint8_t>(hash.bytes, hash.bytes + 32);
}

std::string GetFunctionHash(const std::string& input) {
  std::string result = KeccakHash(input);
  return result.substr(0, std::min(static_cast<size_t>(10), result.length()));
}

// Pads a hex encoded parameter to 32-bytes
// i.e. 64 hex characters.
bool PadHexEncodedParameter(const std::string& hex_input, std::string* out) {
  if (!out) {
    return false;
  }
  if (!IsValidHexString(hex_input)) {
    return false;
  }
  if (hex_input.length() >= 64 + 2) {
    *out = hex_input;
    return true;
  }
  std::string hex_substr = hex_input.substr(2);
  size_t padding_len = 64 - hex_substr.length();
  std::string padding(padding_len, '0');

  *out = "0x" + padding + hex_substr;
  return true;
}

// Determines if the passed in hex string is valid
bool IsValidHexString(const std::string& hex_input) {
  if (hex_input.length() < 3) {
    return false;
  }
  if (!base::StartsWith(hex_input, "0x")) {
    return false;
  }
  for (const auto& c : hex_input.substr(2)) {
    if (!base::IsHexDigit(c)) {
      return false;
    }
  }
  return true;
}

// Takes 2 inputs prefixed by 0x and combines them into an output with a single
// 0x. For example 0x1 and 0x2 would return 0x12
bool ConcatHexStrings(const std::string& hex_input1,
                      const std::string& hex_input2,
                      std::string* out) {
  if (!out) {
    return false;
  }
  if (!IsValidHexString(hex_input1) || !IsValidHexString(hex_input2)) {
    return false;
  }
  *out = hex_input1 + hex_input2.substr(2);
  return true;
}

bool ConcatHexStrings(const std::vector<std::string>& hex_inputs,
                      std::string* out) {
  if (!out) {
    return false;
  }
  if (hex_inputs.empty()) {
    return false;
  }
  if (!IsValidHexString(hex_inputs[0])) {
    return false;
  }

  *out = hex_inputs[0];
  for (size_t i = 1; i < hex_inputs.size(); i++) {
    if (!IsValidHexString(hex_inputs[i])) {
      return false;
    }
    *out += hex_inputs[i].substr(2);
  }

  return true;
}

// Takes a hex string as input and converts it to a uint256_t
bool HexValueToUint256(const std::string& hex_input, uint256_t* out) {
  if (!out) {
    return false;
  }
  if (!IsValidHexString(hex_input)) {
    return false;
  }
  *out = 0;
  for (char c : hex_input.substr(2)) {
    (*out) <<= 4;
    (*out) += static_cast<uint256_t>(base::HexDigitToInt(c));
  }
  return true;
}

// Takes a uint256_t and converts it to a hex string
std::string Uint256ValueToHex(uint256_t input) {
  std::string result;
  result.reserve(32);

  static constexpr char kHexChars[] = "0123456789abcdef";
  while (input) {
    uint8_t i = static_cast<uint8_t>(input & static_cast<uint256_t>(0x0F));
    result.insert(result.begin(), kHexChars[i]);
    input >>= 4;
  }
  if (result.empty()) {
    return "0x0";
  }
  return "0x" + result;
}

std::string GenerateMnemonic(size_t entropy_size) {
  if (!IsValidEntropySize(entropy_size))
    return "";

  std::vector<uint8_t> entropy(entropy_size);
  crypto::RandBytes(&entropy[0], entropy.size());

  return GenerateMnemonicInternal(entropy.data(), entropy.size());
}

std::string GenerateMnemonicForTest(const std::vector<uint8_t>& entropy) {
  return GenerateMnemonicInternal(const_cast<uint8_t*>(entropy.data()),
                                  entropy.size());
}

std::unique_ptr<std::vector<uint8_t>> MnemonicToSeed(
    const std::string& mnemonic,
    const std::string& passphrase) {
  if (!IsValidMnemonic(mnemonic))
    return nullptr;

  std::unique_ptr<std::vector<uint8_t>> seed =
      std::make_unique<std::vector<uint8_t>>(64);
  const std::string salt = "mnemonic" + passphrase;
  int rv = PKCS5_PBKDF2_HMAC(mnemonic.data(), mnemonic.length(),
                             reinterpret_cast<const uint8_t*>(salt.data()),
                             salt.length(), 2048, EVP_sha512(), seed->size(),
                             seed->data());
  return rv == 1 ? std::move(seed) : nullptr;
}

std::unique_ptr<std::vector<uint8_t>> MnemonicToEntropy(
    const std::string& mnemonic) {
  if (!IsValidMnemonic(mnemonic))
    return nullptr;

  const std::vector<std::string> words = SplitString(
      mnemonic, " ", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  // size in bytes
  size_t entropy_size = 0;
  switch (words.size()) {
    case 12:
      entropy_size = 16;
      break;
    case 15:
      entropy_size = 20;
      break;
    case 18:
      entropy_size = 24;
      break;
    case 21:
      entropy_size = 28;
      break;
    case 24:
      entropy_size = 32;
      break;
    default:
      NOTREACHED();
  }
  DCHECK(IsValidEntropySize(entropy_size)) << entropy_size;

  std::unique_ptr<std::vector<uint8_t>> entropy =
      std::make_unique<std::vector<uint8_t>>(entropy_size);

  size_t written;
  if (bip39_mnemonic_to_bytes(nullptr, mnemonic.c_str(), entropy->data(),
                              entropy->size(), &written) != WALLY_OK) {
    LOG(ERROR) << "bip39_mnemonic_to_bytes failed";
    return nullptr;
  }
  return entropy;
}

bool IsValidMnemonic(const std::string& mnemonic) {
  if (bip39_mnemonic_validate(nullptr, mnemonic.c_str()) != WALLY_OK) {
    LOG(ERROR) << __func__ << ": Invalid mnemonic: " << mnemonic;
    return false;
  }
  return true;
}

bool EncodeString(const std::string& input, std::string* output) {
  if (!base::IsStringUTF8(input))
    return false;

  if (input.empty()) {
    *output =
        "0x0000000000000000000000000000000000000000000000000000000000000000";
    return true;
  }

  // Encode count for this string
  bool success =
      PadHexEncodedParameter(Uint256ValueToHex(input.size()), output);
  if (!success)
    return false;

  // Encode string.
  *output += base::ToLowerASCII(base::HexEncode(input.data(), input.size()));

  // Pad 0 to right.
  size_t last_row_len = input.size() % 32;
  if (last_row_len == 0) {
    return true;
  }

  size_t padding_len = (32 - last_row_len) * 2;
  *output += std::string(padding_len, '0');
  return true;
}

bool EncodeStringArray(const std::vector<std::string>& input,
                       std::string* output) {
  // Write count of elements.
  bool success = PadHexEncodedParameter(
      Uint256ValueToHex(static_cast<uint256_t>(input.size())), output);
  if (!success)
    return false;

  // Write offsets to array elements.
  size_t data_offset = input.size() * 32;  // Offset to first element.
  std::string encoded_offset;
  success =
      PadHexEncodedParameter(Uint256ValueToHex(data_offset), &encoded_offset);
  if (!success)
    return false;
  *output += encoded_offset.substr(2);

  for (size_t i = 1; i < input.size(); i++) {
    // Offset for ith element =
    //     offset for i-1th + 32 * (count for i-1th) +
    //     32 * ceil(i-1th.size() / 32.0) (length of encoding for i-1th).
    std::string encoded_offset;
    size_t rows = std::ceil(input[i - 1].size() / 32.0);
    data_offset += (rows + 1) * 32;

    success =
        PadHexEncodedParameter(Uint256ValueToHex(data_offset), &encoded_offset);
    if (!success)
      return false;
    *output += encoded_offset.substr(2);
  }

  // Write count and encoding for array elements.
  for (size_t i = 0; i < input.size(); i++) {
    std::string encoded_string;
    success = EncodeString(input[i], &encoded_string);
    if (!success)
      return false;
    *output += encoded_string.substr(2);
  }

  return true;
}

bool DecodeString(size_t offset,
                  const std::string& input,
                  std::string* output) {
  if (!output->empty())
    return false;

  // Decode count.
  uint256_t count = 0;
  size_t len = 64;
  if (offset + len > input.size() ||
      !HexValueToUint256("0x" + input.substr(offset, len), &count)) {
    return false;
  }

  // Empty string case.
  if (!count) {
    *output = "";
    return true;
  }

  // Decode string.
  offset += len;
  len = static_cast<size_t>(count) * 2;
  return offset + len <= input.size() &&
         base::HexStringToString(input.substr(offset, len), output);
}

bool DecodeStringArray(const std::string& input,
                       std::vector<std::string>* output) {
  // Get count of array.
  uint256_t count = 0;
  if (!HexValueToUint256("0x" + input.substr(0, 64), &count)) {
    return false;
  }

  // Decode count and string for each array element.
  *output = std::vector<std::string>(static_cast<size_t>(count), "");
  size_t offset = 64;  // Offset to count of first element.
  for (size_t i = 0; i < static_cast<size_t>(count); i++) {
    // Get the starting data offset for each string element.
    uint256_t data_offset;
    if (offset + 64 > input.size() ||
        !HexValueToUint256("0x" + input.substr(offset, 64), &data_offset)) {
      return false;
    }

    // Decode each string.
    size_t string_offset =
        64 /* count */ + static_cast<size_t>(data_offset) * 2;
    if (string_offset > input.size() ||
        !DecodeString(string_offset, input, &output->at(i))) {
      return false;
    }

    offset += 64;  // Offset for next count.
  }

  return true;
}

std::string Namehash(const std::string& name) {
  std::string hash(32, '\0');
  std::vector<std::string> labels =
      SplitString(name, ".", base::KEEP_WHITESPACE, base::SPLIT_WANT_NONEMPTY);

  for (auto rit = labels.rbegin(); rit != labels.rend(); rit++) {
    std::string label_hash = KeccakHash(*rit, false);
    hash = KeccakHash(hash + label_hash, false);
  }

  return ToHex(hash);
}

void SecureZeroData(void* data, size_t size) {
  if (data == nullptr || size == 0)
    return;
#if defined(OS_WIN)
  SecureZeroMemory(data, size);
#else
  // 'volatile' is required. Otherwise optimizers can remove this function
  // if cleaning local variables, which are not used after that.
  volatile uint8_t* d = (volatile uint8_t*)data;
  for (size_t i = 0; i < size; i++)
    d[i] = 0;
#endif
}

// Updates preferences for when the wallet is unlocked.
// This is done in a utils function instead of in the KeyringController
// because we call it both from the old extension and the new wallet when
// it unlocks.
void UpdateLastUnlockPref(PrefService* prefs) {
  prefs->SetTime(kBraveWalletLastUnlockTime, base::Time::Now());
}

base::Value TransactionReceiptToValue(const TransactionReceipt& tx_receipt) {
  base::Value dict(base::Value::Type::DICTIONARY);
  dict.SetStringKey("transaction_hash", tx_receipt.transaction_hash);
  dict.SetStringKey("transaction_index",
                    Uint256ValueToHex(tx_receipt.transaction_index));
  dict.SetStringKey("block_hash", tx_receipt.block_hash);
  dict.SetStringKey("block_number", Uint256ValueToHex(tx_receipt.block_number));
  dict.SetStringKey("from", tx_receipt.from);
  dict.SetStringKey("to", tx_receipt.to);
  dict.SetStringKey("cumulative_gas_used",
                    Uint256ValueToHex(tx_receipt.cumulative_gas_used));
  dict.SetStringKey("gas_used", Uint256ValueToHex(tx_receipt.gas_used));
  dict.SetStringKey("contract_address", tx_receipt.contract_address);
  // TODO(darkdh): logs
  dict.SetStringKey("logs_bloom", tx_receipt.logs_bloom);
  dict.SetBoolKey("status", tx_receipt.status);
  return dict;
}

absl::optional<TransactionReceipt> ValueToTransactionReceipt(
    const base::Value& value) {
  TransactionReceipt tx_receipt;
  const std::string* transaction_hash = value.FindStringKey("transaction_hash");
  if (!transaction_hash)
    return absl::nullopt;
  tx_receipt.transaction_hash = *transaction_hash;

  const std::string* transaction_index =
      value.FindStringKey("transaction_index");
  if (!transaction_index)
    return absl::nullopt;
  uint256_t transaction_index_uint;
  if (!HexValueToUint256(*transaction_index, &transaction_index_uint))
    return absl::nullopt;
  tx_receipt.transaction_index = transaction_index_uint;

  const std::string* block_hash = value.FindStringKey("block_hash");
  if (!block_hash)
    return absl::nullopt;
  tx_receipt.block_hash = *block_hash;

  const std::string* block_number = value.FindStringKey("block_number");
  if (!block_number)
    return absl::nullopt;
  uint256_t block_number_uint;
  if (!HexValueToUint256(*block_number, &block_number_uint))
    return absl::nullopt;
  tx_receipt.block_number = block_number_uint;

  const std::string* from = value.FindStringKey("from");
  if (!from)
    return absl::nullopt;
  tx_receipt.from = *from;

  const std::string* to = value.FindStringKey("to");
  if (!to)
    return absl::nullopt;
  tx_receipt.to = *to;

  const std::string* cumulative_gas_used =
      value.FindStringKey("cumulative_gas_used");
  if (!cumulative_gas_used)
    return absl::nullopt;
  uint256_t cumulative_gas_used_uint;
  if (!HexValueToUint256(*cumulative_gas_used, &cumulative_gas_used_uint))
    return absl::nullopt;
  tx_receipt.cumulative_gas_used = cumulative_gas_used_uint;

  const std::string* gas_used = value.FindStringKey("gas_used");
  if (!gas_used)
    return absl::nullopt;
  uint256_t gas_used_uint;
  if (!HexValueToUint256(*gas_used, &gas_used_uint))
    return absl::nullopt;
  tx_receipt.gas_used = gas_used_uint;

  const std::string* contract_address = value.FindStringKey("contract_address");
  if (!contract_address)
    return absl::nullopt;
  tx_receipt.contract_address = *contract_address;

  // TODO(darkdh): logs
  const std::string* logs_bloom = value.FindStringKey("logs_bloom");
  if (!logs_bloom)
    return absl::nullopt;
  tx_receipt.logs_bloom = *logs_bloom;

  absl::optional<bool> status = value.FindBoolKey("status");
  if (!status)
    return absl::nullopt;
  tx_receipt.status = *status;

  return tx_receipt;
}

void GetAllKnownChains(PrefService* prefs,
                       std::vector<mojom::EthereumChainPtr>* chains) {
  for (const auto& network : kKnownNetworks) {
    chains->push_back(GetKnownChain(prefs, network.chain_id));
  }
}

GURL GetNetworkURL(PrefService* prefs, const std::string& chain_id) {
  mojom::EthereumChainPtr known_network = GetKnownChain(prefs, chain_id);
  if (!known_network)
    return GetCustomChainURL(prefs, chain_id);

  if (known_network->rpc_urls.size())
    return GURL(known_network->rpc_urls.front());

  return GURL();
}

void GetAllChains(PrefService* prefs,
                  std::vector<mojom::EthereumChainPtr>* result) {
  GetAllKnownChains(prefs, result);
  GetAllCustomChains(prefs, result);
}

std::vector<std::string> GetAllKnownNetworkIds() {
  std::vector<std::string> network_ids;
  for (const auto& network : kKnownNetworks) {
    std::string network_id = GetKnownNetworkId(network.chain_id);
    if (!network_id.empty())
      network_ids.push_back(network_id);
  }
  return network_ids;
}

std::string GetKnownNetworkId(const std::string& chain_id) {
  auto subdomain = GetInfuraSubdomainForKnownChainId(chain_id);
  if (!subdomain.empty())
    return subdomain;

  // Separate check for localhost in known networks as it is predefined but
  // does not have infura subdomain.
  if (chain_id == mojom::kLocalhostChainId) {
    for (const auto& network : kKnownNetworks) {
      if (network.chain_id == chain_id) {
        return GURL(network.rpc_urls.front()).spec();
      }
    }
  }

  return "";
}

std::string GetNetworkId(PrefService* prefs, const std::string& chain_id) {
  DCHECK(prefs);

  std::string id = GetKnownNetworkId(chain_id);
  if (!id.empty())
    return id;

  std::vector<mojom::EthereumChainPtr> custom_chains;
  GetAllCustomChains(prefs, &custom_chains);
  for (const auto& network : custom_chains) {
    if (network->chain_id != chain_id)
      continue;
    if (network->rpc_urls.size()) {
      id = GURL(network->rpc_urls.front()).host();
    } else {
      id = chain_id;
    }
    break;
  }

  return id;
}

mojom::DefaultWallet GetDefaultWallet(PrefService* prefs) {
  return static_cast<brave_wallet::mojom::DefaultWallet>(
      prefs->GetInteger(kBraveWalletWeb3Provider));
}

void SetDefaultWallet(PrefService* prefs, mojom::DefaultWallet default_wallet) {
  prefs->SetInteger(kBraveWalletWeb3Provider, static_cast<int>(default_wallet));
}

std::string GetUnstoppableDomainsProxyReaderContractAddress(
    const std::string& chain_id) {
  if (kUnstoppableDomainsProxyReaderContractAddressMap.contains(chain_id))
    return kUnstoppableDomainsProxyReaderContractAddressMap.at(chain_id);
  return "";
}

std::string GetEnsRegistryContractAddress(const std::string& chain_id) {
  if (kEnsRegistryContractAddressMap.contains(chain_id))
    return kEnsRegistryContractAddressMap.at(chain_id);
  return "";
}

}  // namespace brave_wallet
