/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DPipelineState_DEFINED
#define GrD3DPipelineState_DEFINED

#include "include/gpu/GrTypes.h"
#include "src/gpu/d3d/GrD3D12.h"
#include <memory>

class GrProgramInfo;

class GrD3DPipelineState {
public:
    static std::unique_ptr<GrD3DPipelineState> Make(const GrProgramInfo&);

private:
    GrD3DPipelineState(gr_cp<ID3D12PipelineState> pipelineState);

    gr_cp<ID3D12PipelineState> fPipelineState;

};

#endif
