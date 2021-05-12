use std::{
    env,
    path::PathBuf,
};

fn main() {
    // TODO: make this generic & work on bot macOS & Windows
    println!("cargo:rerun-if-changed=wrapper.hpp");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    /*let output = if cfg!(target_os = "windows") {
        Command::new("sh")
            .arg("-c")
            .arg("echo hello")
            .output()
            .expect("failed to execute process")
    } else {
        Command::new("make")
            .args(&["--directory", "opennurbs"])
            .output()
            .expect("Failed to build OpenNURBS.")
    };*/

    let project_path = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    let mut ayam_path = project_path;
    ayam_path.push("ayam");
    ayam_path.push("ayam");
    ayam_path.push("src");
    let mut ayam_nurbs_path = ayam_path.clone();
    ayam_nurbs_path.push("nurbs");

    let mut ayam_togl_path = ayam_path.clone();
    ayam_togl_path.push("togl");

    println!("{}", &ayam_togl_path.display());

    let mut ayam_affine_path = ayam_path.clone();
    ayam_affine_path.push("affine");
    ayam_affine_path.push("include");

    let nurbs_files = [
        "apt.c",
        "bevelt.c",
        "capt.c",
        "ict.c",
        "ipt.c",
        "knots.c",
        "nb.c",
        "nct.c",
        "npt.c",
        "pmt.c",
        "stess.c",
        "tess.c",
    ];

    let mut ayam_build = cc::Build::new();

    ayam_build
        .include(&ayam_path)
        .include(&ayam_affine_path)
        .include(&ayam_nurbs_path)
        .include(&ayam_togl_path)
        .files(nurbs_files.iter().map(|file| {
            let mut path = ayam_nurbs_path.clone();
            path.push(file);
            path
        }))
        .warnings(false)
        .opt_level(3)
        .define("AYUSEAFFINE", None);

    //#[cfg(target_os = "macos")]
    //ayam_build.define(AYWITHAQUA, None);

    ayam_build.compile("ayan");

    /*#[cfg(target_os = "linux")]
    println!("cargo:rustc-link-lib=dylib=stdc++");
    #[cfg(target_os = "macos")]
    println!("cargo:rustc-link-lib=dylib=c++");*/

    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        //.derive_debug(true)
        //.whitelist_function("ON_.*")
        //.allowlist_type("ON_.*")
        //.allowlist_var("ON_.*")
        //.allowlist_type("ONX_.*")
        .allowlist_var("ay_.*")
        .allowlist_type("ay_.*")
        .allowlist_function("ay_.*")
        //.whitelist_var("kSP.*")
        .clang_arg(format!("-I{}", ayam_path.display()))
        .clang_arg(format!("-I{}", &ayam_affine_path.display()))
        .clang_arg(format!("-I{}", &ayam_nurbs_path.display()))
        .clang_arg(format!("-I{}", &ayam_togl_path.display()))
        .clang_arg("-DAYUSEAFFINE")
        //.clang_arg("-std=c++14")
        //.generate_inline_functions(true)
        .rustified_enum("*")
        //.clang_arg(format!("-I{}", Path::new( ae_sdk_path ).join("Examples").join("Headers").join("SP").display()))
        //.clang_arg(format!("-I{}", Path::new( ae_sdk_path ).join("Examples").join("Util").display()))
        //#[cfg(target_os = "macos")]
        //.clang_arg("-I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers/")
        //.clang_arg("-I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Headers/")
        //.clang_arg("-I/Library/Developer/CommandLineTools/usr/include/c++/v1/")
        //.clang_arg("-F/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Unable to generate Ayam bindings");


    // Write the bindings to the $OUT_DIR/bindings.rs file.
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Unable to write Ayam bindings");


}
