// Tests fuer Signaling-Bits-Serialisierung + Linear Value-Buffer mit Lazy Deletion
// REV 6 §5.17

#include <prt_art/serialization/signaling_bits.hpp>
#include <prt_art/value_buffer/linear_value_buffer.hpp>

#include <gtest/gtest.h>

#include <array>
#include <vector>

namespace ser = comdare::prt_art::serialization;
namespace buf = comdare::prt_art::value_buffer;

// ─────────────────────────────────────────────────────────────────────────────
// VarLenEncoder
// ─────────────────────────────────────────────────────────────────────────────

TEST(VarLenEncoder, EncodesSmallValuesIn1Byte) {
    std::byte out[9];
    EXPECT_EQ(ser::VarLenEncoder::encode(0,   out, 9), 1u);
    EXPECT_EQ(ser::VarLenEncoder::encode(127, out, 9), 1u);
}

TEST(VarLenEncoder, EncodesLargerValuesInMultipleBytes) {
    std::byte out[9];
    EXPECT_EQ(ser::VarLenEncoder::encode(128,  out, 9), 2u);
    EXPECT_EQ(ser::VarLenEncoder::encode(16384, out, 9), 3u);
}

TEST(VarLenEncoder, EncodeDecodeRoundtrip) {
    std::byte out[9];
    for (std::uint64_t v : {0u, 1u, 127u, 128u, 16383u, 16384u, 1u<<20, 1u<<28}) {
        std::size_t n = ser::VarLenEncoder::encode(v, out, 9);
        std::span<std::byte const> in(out, n);
        auto d = ser::VarLenEncoder::decode(in);
        EXPECT_EQ(d.value, v) << "for v=" << v;
        EXPECT_EQ(d.consumed_bytes, n);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SignalingStream
// ─────────────────────────────────────────────────────────────────────────────

TEST(SignalingStream, EmptyOnConstruction) {
    ser::SignalingStream s;
    EXPECT_EQ(s.entry_count(), 0u);
    EXPECT_EQ(s.byte_size(), 0u);
}

TEST(SignalingStream, AppendNormalEntry) {
    ser::SignalingStream s;
    std::vector<std::byte> payload{std::byte{1}, std::byte{2}, std::byte{3}};
    s.append(ser::SignalKind::Normal, std::span<std::byte const>(payload.data(), payload.size()));
    EXPECT_EQ(s.entry_count(), 1u);
    EXPECT_GT(s.byte_size(), 3u);   // Signal + Length-Header + Payload
}

TEST(SignalingStream, DecodeRoundtripSingleEntry) {
    ser::SignalingStream s;
    std::vector<std::byte> payload{std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC}};
    s.append(ser::SignalKind::Special, std::span<std::byte const>(payload.data(), payload.size()));

    auto e = ser::SignalingStream::decode_one(s.raw(), 0);
    EXPECT_EQ(e.signal, ser::SignalKind::Special);
    ASSERT_EQ(e.payload.size(), 3u);
    EXPECT_EQ(e.payload[0], std::byte{0xAA});
    EXPECT_EQ(e.payload[2], std::byte{0xCC});
    EXPECT_GT(e.next_offset, 0u);
}

TEST(SignalingStream, MultipleEntriesIterable) {
    ser::SignalingStream s;
    std::vector<std::byte> a{std::byte{1}};
    std::vector<std::byte> b{std::byte{2}, std::byte{3}};
    s.append(ser::SignalKind::Normal,  std::span<std::byte const>(a.data(), a.size()));
    s.append(ser::SignalKind::Special, std::span<std::byte const>(b.data(), b.size()));

    auto raw = s.raw();
    auto e1 = ser::SignalingStream::decode_one(raw, 0);
    auto e2 = ser::SignalingStream::decode_one(raw, e1.next_offset);
    EXPECT_EQ(e1.signal, ser::SignalKind::Normal);
    EXPECT_EQ(e1.payload.size(), 1u);
    EXPECT_EQ(e2.signal, ser::SignalKind::Special);
    EXPECT_EQ(e2.payload.size(), 2u);
}

TEST(SignalingStream, ClearResets) {
    ser::SignalingStream s;
    std::vector<std::byte> p{std::byte{1}};
    s.append(ser::SignalKind::Normal, std::span<std::byte const>(p.data(), p.size()));
    s.clear();
    EXPECT_EQ(s.entry_count(), 0u);
    EXPECT_EQ(s.byte_size(), 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// LinearValueBuffer (Lazy Deletion)
// ─────────────────────────────────────────────────────────────────────────────

TEST(LinearValueBuffer, EmptyOnConstruction) {
    buf::LinearValueBuffer b;
    EXPECT_EQ(b.total_records(), 0u);
    EXPECT_EQ(b.alive_records(), 0u);
    EXPECT_EQ(b.tombstone_records(), 0u);
}

TEST(LinearValueBuffer, AppendAndRead) {
    buf::LinearValueBuffer b;
    std::vector<std::byte> v{std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};
    auto id = b.append(std::span<std::byte const>(v.data(), v.size()));
    EXPECT_EQ(b.total_records(), 1u);
    EXPECT_EQ(b.alive_records(), 1u);

    auto read = b.read(id);
    ASSERT_EQ(read.size(), 3u);
    EXPECT_EQ(read[0], std::byte{0x11});
    EXPECT_EQ(read[2], std::byte{0x33});
}

TEST(LinearValueBuffer, EraseMarksTombstone) {
    buf::LinearValueBuffer b;
    std::vector<std::byte> v{std::byte{0xAA}};
    auto id = b.append(std::span<std::byte const>(v.data(), v.size()));
    b.erase(id);
    EXPECT_FALSE(b.is_alive(id));
    EXPECT_EQ(b.tombstone_records(), 1u);
    EXPECT_EQ(b.alive_records(), 0u);
    EXPECT_TRUE(b.read(id).empty());
}

TEST(LinearValueBuffer, EraseTwiceIsSafe) {
    buf::LinearValueBuffer b;
    std::vector<std::byte> v{std::byte{1}};
    auto id = b.append(std::span<std::byte const>(v.data(), v.size()));
    b.erase(id);
    b.erase(id);
    EXPECT_EQ(b.tombstone_records(), 1u);
}

TEST(LinearValueBuffer, ShouldCompactBelowThreshold) {
    buf::LinearValueBuffer b;
    std::vector<std::byte> v(100, std::byte{0});
    for (int i = 0; i < 5; ++i) b.append(std::span<std::byte const>(v.data(), v.size()));
    b.erase(0);   // 100/500 = 20% tombstone
    EXPECT_FALSE(b.should_compact(0.30));
}

TEST(LinearValueBuffer, ShouldCompactAboveThreshold) {
    buf::LinearValueBuffer b;
    std::vector<std::byte> v(100, std::byte{0});
    for (int i = 0; i < 5; ++i) b.append(std::span<std::byte const>(v.data(), v.size()));
    b.erase(0); b.erase(1);   // 200/500 = 40% tombstone
    EXPECT_TRUE(b.should_compact(0.30));
}

TEST(LinearValueBuffer, CompactRemovesTombstonesAndRemaps) {
    buf::LinearValueBuffer b;
    std::vector<std::byte> v1{std::byte{1}};
    std::vector<std::byte> v2{std::byte{2}};
    std::vector<std::byte> v3{std::byte{3}};
    auto id1 = b.append(std::span<std::byte const>(v1.data(), v1.size()));
    auto id2 = b.append(std::span<std::byte const>(v2.data(), v2.size()));
    auto id3 = b.append(std::span<std::byte const>(v3.data(), v3.size()));
    b.erase(id2);

    auto mapping = b.compact();
    ASSERT_EQ(mapping.size(), 3u);
    EXPECT_NE(mapping[id1], static_cast<std::uint64_t>(-1));   // erhalten
    EXPECT_EQ(mapping[id2], static_cast<std::uint64_t>(-1));   // tombstone → unmapped
    EXPECT_NE(mapping[id3], static_cast<std::uint64_t>(-1));

    EXPECT_EQ(b.total_records(), 2u);
    EXPECT_EQ(b.tombstone_records(), 0u);
    EXPECT_EQ(b.alive_records(), 2u);
    EXPECT_EQ(b.read(mapping[id1]).size(), 1u);
    EXPECT_EQ(b.read(mapping[id3]).size(), 1u);
}
