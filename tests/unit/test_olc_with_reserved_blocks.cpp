// Tests fuer OLC + reservierte Value-Bloecke (REV 6 §5.17)

#include <prt_art/concurrency/olc_with_reserved_blocks.hpp>

#include <gtest/gtest.h>

namespace conc = comdare::prt_art::concurrency;

TEST(OlcWithReservedValueBlocks, InitialVersionIsZero) {
    conc::OlcWithReservedValueBlocks lock;
    EXPECT_EQ(lock.read_version(), 0u);
    EXPECT_FALSE(conc::OlcWithReservedValueBlocks::is_write_in_progress(lock.read_version()));
}

TEST(OlcWithReservedValueBlocks, ValidateUnchangedVersion) {
    conc::OlcWithReservedValueBlocks lock;
    auto v = lock.read_version();
    EXPECT_TRUE(lock.validate(v));
}

TEST(OlcWithReservedValueBlocks, BeginWriteSetsWriteMarker) {
    conc::OlcWithReservedValueBlocks lock;
    lock.begin_write();
    EXPECT_TRUE(conc::OlcWithReservedValueBlocks::is_write_in_progress(lock.read_version()));
}

TEST(OlcWithReservedValueBlocks, EndWriteIncrementsAndClearsMarker) {
    conc::OlcWithReservedValueBlocks lock;
    auto v0 = lock.read_version();
    lock.begin_write();
    lock.end_write();
    auto v1 = lock.read_version();
    EXPECT_FALSE(conc::OlcWithReservedValueBlocks::is_write_in_progress(v1));
    EXPECT_EQ(v1, v0 + conc::OlcWithReservedValueBlocks::kIncrementStep);
}

TEST(OlcWithReservedValueBlocks, ValidateFailsAfterWrite) {
    conc::OlcWithReservedValueBlocks lock;
    auto captured = lock.read_version();
    lock.begin_write();
    lock.end_write();
    EXPECT_FALSE(lock.validate(captured));
}

TEST(OlcWithReservedValueBlocks, ReserveBlockReturnsUniqueIds) {
    conc::OlcWithReservedValueBlocks lock;
    auto a = lock.reserve_value_block();
    auto b = lock.reserve_value_block();
    auto c = lock.reserve_value_block();
    EXPECT_EQ(a, 0u);
    EXPECT_EQ(b, 1u);
    EXPECT_EQ(c, 2u);
    EXPECT_EQ(lock.total_blocks_reserved(), 3u);
}

TEST(OlcWithReservedValueBlocks, ResetClearsState) {
    conc::OlcWithReservedValueBlocks lock;
    lock.begin_write();
    lock.end_write();
    (void)lock.reserve_value_block();
    lock.reset();
    EXPECT_EQ(lock.read_version(), 0u);
    EXPECT_EQ(lock.total_blocks_reserved(), 0u);
}

// WriteGuard RAII
TEST(WriteGuard, AcquiresAndReleasesAutomatically) {
    conc::OlcWithReservedValueBlocks lock;
    auto v0 = lock.read_version();
    {
        conc::WriteGuard g{lock};
        EXPECT_TRUE(conc::OlcWithReservedValueBlocks::is_write_in_progress(lock.read_version()));
        EXPECT_EQ(g.reserved_block_id(), 0u);
    }
    auto v1 = lock.read_version();
    EXPECT_FALSE(conc::OlcWithReservedValueBlocks::is_write_in_progress(v1));
    EXPECT_GT(v1, v0);
}

TEST(WriteGuard, EachGuardReservesNewBlock) {
    conc::OlcWithReservedValueBlocks lock;
    {
        conc::WriteGuard g1{lock};
        EXPECT_EQ(g1.reserved_block_id(), 0u);
    }
    {
        conc::WriteGuard g2{lock};
        EXPECT_EQ(g2.reserved_block_id(), 1u);
    }
    EXPECT_EQ(lock.total_blocks_reserved(), 2u);
}
