// status.h — Current sensing, overcurrent protection, and diagnostic test modes
#pragma once

#include <Arduino.h>

class Hardware;

/**
 * Provides board diagnostics and protection:
 *   - Analog current measurement
 *   - Hardware overcurrent alert monitoring
 *   - Self-test wrapper
 *   - LED-cycle and DIP-switch echo test modes
 */
class Status {
public:
    /// Maximum allowed current in milliamps before software shutdown.
    static constexpr int MAX_CURRENT_MA = 16000;

    /**
     * @param hw  Reference to the hardware driver.
     */
    explicit Status(Hardware &hw);

    /**
     * Read the board current draw.
     * @return current in milliamps.
     */
    int readCurrentMa() const;

    /**
     * Check the /Overcurrent Alert line and the software current limit.
     * If tripped, all valves are turned off immediately.
     * @return true if overcurrent condition detected.
     */
    bool checkOvercurrent();

    // ── Test / demo modes (selected by DIP switches) ──────────────

    /** Sequentially energise each valve driver (visual walk test). */
    void cycleLeds();

    /** Echo DIP switches 3-4 and spare inputs to selected valve outputs. */
    void showDipSwitches();

private:
    Hardware &hw_;
};
