#pragma once
// SignalingBits — Format: 1 Bit Signal + Variable Length-Bytes + Payload
// REV 6 §5.17 — Signaling-Bits-Serialisierung
//
// Layout pro Eintrag im seriellen Stream:
//   [signal_bit:1] [serialized_length:N Bytes (varlen-encoded)] [payload:length Bytes]
//
// Das signal_bit signalisiert Sonderbedingungen (z.B. tombstone, layout-switch),
// die serialized_length ist varlen-codiert (1-9 Bytes je nach Groesse),
// die Payload ist roh.

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace comdare::prt_art::serialization {

enum class SignalKind : std::uint8_t {
    Normal  = 0, // signal_bit = 0
    Special = 1, // signal_bit = 1 (tombstone, layout-switch, layout-marker)
};

// Varlen-Encoding analog Protobuf-style (7-Bit-Continue): bis 9 Bytes
class VarLenEncoder {
public:
    // Encode size in 1..9 Bytes; gibt Anzahl der geschriebenen Bytes zurueck
    [[nodiscard]] static std::size_t encode(std::uint64_t value, std::byte* out, std::size_t cap) noexcept {
        std::size_t written = 0;
        while (value >= 0x80 && written < cap) {
            out[written++] = static_cast<std::byte>((value & 0x7F) | 0x80);
            value >>= 7;
        }
        if (written < cap) out[written++] = static_cast<std::byte>(value & 0x7F);
        return written;
    }

    // Decode liefert (value, consumed_bytes)
    struct Decoded {
        std::uint64_t value          = 0;
        std::size_t   consumed_bytes = 0;
    };

    [[nodiscard]] static Decoded decode(std::span<std::byte const> in) noexcept {
        Decoded       result{};
        std::uint64_t shift = 0;
        for (std::byte b : in) {
            ++result.consumed_bytes;
            result.value |= static_cast<std::uint64_t>(static_cast<std::uint8_t>(b) & 0x7F) << shift;
            if ((static_cast<std::uint8_t>(b) & 0x80) == 0) break;
            shift += 7;
            if (shift >= 64) {
                result.consumed_bytes = 0; // overflow
                result.value          = 0;
                break;
            }
        }
        return result;
    }
};

class SignalingStream {
public:
    void append(SignalKind signal, std::span<std::byte const> payload) {
        // Reserve worst case
        std::byte   length_buf[9];
        std::size_t length_bytes =
            VarLenEncoder::encode(static_cast<std::uint64_t>(payload.size()), length_buf, sizeof(length_buf));

        // Signal-Bit + Length-Header-First-Byte zusammen?  Wir nutzen ein extra Byte
        // fuer das Signal — pragmatisch + lesbar (Kosten: 1 Byte Overhead pro Eintrag).
        buffer_.push_back(static_cast<std::byte>(static_cast<std::uint8_t>(signal)));
        buffer_.insert(buffer_.end(), length_buf, length_buf + length_bytes);
        buffer_.insert(buffer_.end(), payload.begin(), payload.end());
        ++entry_count_;
    }

    [[nodiscard]] std::span<std::byte const> raw() const noexcept {
        return std::span<std::byte const>(buffer_.data(), buffer_.size());
    }

    [[nodiscard]] std::size_t entry_count() const noexcept { return entry_count_; }
    [[nodiscard]] std::size_t byte_size() const noexcept { return buffer_.size(); }

    void clear() noexcept {
        buffer_.clear();
        entry_count_ = 0;
    }

    // Iterator-style decode
    struct Entry {
        SignalKind                 signal = SignalKind::Normal;
        std::span<std::byte const> payload{};
        std::size_t                next_offset = 0;
    };

    [[nodiscard]] static Entry decode_one(std::span<std::byte const> stream, std::size_t offset) noexcept {
        Entry e{};
        if (offset + 1 > stream.size()) return e;
        e.signal = static_cast<SignalKind>(static_cast<std::uint8_t>(stream[offset]));
        auto sub = stream.subspan(offset + 1);
        auto dec = VarLenEncoder::decode(sub);
        if (dec.consumed_bytes == 0) return e;
        std::size_t payload_start = offset + 1 + dec.consumed_bytes;
        std::size_t payload_end   = payload_start + dec.value;
        if (payload_end > stream.size()) return e;
        e.payload     = stream.subspan(payload_start, dec.value);
        e.next_offset = payload_end;
        return e;
    }

private:
    std::vector<std::byte> buffer_{};
    std::size_t            entry_count_ = 0;
};

} // namespace comdare::prt_art::serialization
