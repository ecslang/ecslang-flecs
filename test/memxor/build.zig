const std = @import("std");

const exe_names = [_][]const u8{
    "cache_info",
    "cpu_info",
};

const exe_cflags = [_][]const u8{
    "-std=c++17",
};

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    for (exe_names) |exe_name| {
        const exe = b.addExecutable(.{
            .name = exe_name,
            .target = target,
            .optimize = optimize,
        });

        exe.addCSourceFile(b.fmt("{s}.cpp", .{exe_name}), &exe_cflags);
        exe.linkLibC();
        exe.linkLibCpp();

        if (target.isWindows() and target.getAbi() == .gnu) {
            // LTO is currently broken on mingw, this can be removed when it's fixed.
            exe.want_lto = false;
        }

        // This declares intent for the executable to be installed into the
        // standard location when the user invokes the "install" step (the default
        // step when running `zig build`).
        b.installArtifact(exe);
    }
}
