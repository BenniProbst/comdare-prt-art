// Tests fuer OLC + reservierte Value-Bloecke (REV 6 §5.17)

#include <prt_art/concurrency/olc_with_reserved_blocks.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

namespace conc = comdare::prt_art::concurrency;

TEST(OlcWithReservedValueBlocks, InitialVersionIsZero) {
    conc::OlcWithReservedValueBlocks lock;
    EXPECT_EQ(lock.read_version(), 0u);
    EXPECT_FALSE(conc::OlcWithReservedValueBlocks::is_write_in_progress(lock.read_version()));
}

TEST(OlcWithReservedValueBlocks, ValidateUnchangedVersion) {
    conc::OlcWithReservedValueBlocks lock;
    auto                             v = lock.read_version();
    EXPECT_TRUE(lock.validate(v));
}

TEST(OlcWithReservedValueBlocks, BeginWriteSetsWriteMarker) {
    conc::OlcWithReservedValueBlocks lock;
    lock.begin_write();
    EXPECT_TRUE(conc::OlcWithReservedValueBlocks::is_write_in_progress(lock.read_version()));
}

TEST(OlcWithReservedValueBlocks, EndWriteIncrementsAndClearsMarker) {
    conc::OlcWithReservedValueBlocks lock;
    auto                             v0 = lock.read_version();
    lock.begin_write();
    lock.end_write();
    auto v1 = lock.read_version();
    EXPECT_FALSE(conc::OlcWithReservedValueBlocks::is_write_in_progress(v1));
    EXPECT_EQ(v1, v0 + conc::OlcWithReservedValueBlocks::kIncrementStep);
}

TEST(OlcWithReservedValueBlocks, ValidateFailsAfterWrite) {
    conc::OlcWithReservedValueBlocks lock;
    auto                             captured = lock.read_version();
    lock.begin_write();
    lock.end_write();
    EXPECT_FALSE(lock.validate(captured));
}

TEST(OlcWithReservedValueBlocks, ReserveBlockReturnsUniqueIds) {
    conc::OlcWithReservedValueBlocks lock;
    auto                             a = lock.reserve_value_block();
    auto                             b = lock.reserve_value_block();
    auto                             c = lock.reserve_value_block();
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
    auto                             v0 = lock.read_version();
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

// ─────────────────────────────────────────────────────────────────────────────
// (REV-CXX-02, WP-5 2026-07-16): Odd/Even-Vertrag unter MEHREREN Writern.
// Vorher (fetch_add): Writer B kippte die von Writer A ungerade gesetzte Version
// zurueck auf gerade — ein optimistischer Leser sah "kein Write" trotz zweier
// aktiver Writer. Jetzt (CAS-Schleife): solange irgendein Writer aktiv ist, ist
// die Version UNGERADE; ein zweiter Writer tritt erst nach end_write des ersten
// ein. Diese Tests sind nicht timing-flaky: b_entered kann nur true werden, wenn
// begin_write zurueckkehrte — was waehrend eines aktiven Writers unmoeglich ist.
// ─────────────────────────────────────────────────────────────────────────────
TEST(OlcWithReservedValueBlocks, SecondWriterCannotEnterWhileFirstActive) {
    conc::OlcWithReservedValueBlocks lock;
    auto const                       v0 = lock.read_version();

    lock.begin_write(); // Writer A aktiv -> Version ungerade
    std::atomic<bool> b_entered{false};
    std::thread       writer_b([&] {
        lock.begin_write(); // muss blockieren, bis A end_write ruft
        b_entered.store(true, std::memory_order_release);
        EXPECT_TRUE(conc::OlcWithReservedValueBlocks::is_write_in_progress(lock.read_version()));
        lock.end_write();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(b_entered.load(std::memory_order_acquire)); // B haengt korrekt vor dem Vertrag
    // Waehrend A (und ggf. wartendem B) ist KEINE valide gerade Version sichtbar:
    EXPECT_TRUE(conc::OlcWithReservedValueBlocks::is_write_in_progress(lock.read_version()));
    EXPECT_FALSE(lock.validate(v0));

    lock.end_write(); // A publiziert die naechste gerade Generation -> B tritt ein
    writer_b.join();

    auto const v_final = lock.read_version();
    EXPECT_FALSE(conc::OlcWithReservedValueBlocks::is_write_in_progress(v_final));
    EXPECT_EQ(v_final, v0 + 2u * conc::OlcWithReservedValueBlocks::kIncrementStep); // monoton, 2 Writes
}

TEST(OlcWithReservedValueBlocks, ConcurrentWritersKeepMonotoneEvenGenerations) {
    conc::OlcWithReservedValueBlocks lock;
    constexpr int                    kIters = 2000;
    auto                             writer = [&] {
        for (int i = 0; i < kIters; ++i) {
            lock.begin_write();
            lock.end_write();
        }
    };
    std::thread t1(writer);
    std::thread t2(writer);
    t1.join();
    t2.join();
    auto const v_final = lock.read_version();
    EXPECT_FALSE(conc::OlcWithReservedValueBlocks::is_write_in_progress(v_final));
    EXPECT_EQ(v_final, 2u * kIters * conc::OlcWithReservedValueBlocks::kIncrementStep);
}
