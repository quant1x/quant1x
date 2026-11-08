// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package quant1x.distributed.id;

import java.io.IOException;
import java.util.Optional;

interface StateStore {
    Optional<PersistentState> load() throws IOException;

    PersistentState next(PersistentState local, long nowMs, int seqBits) throws IOException;
}
