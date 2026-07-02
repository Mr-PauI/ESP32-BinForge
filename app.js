document.addEventListener("DOMContentLoaded", () => {

    let state = {
        file: null,
        binaryData: null,
        binarySize: 0
    };

    let ModuleInstance = null;

    const fileInput = document.getElementById("fileInput");
    const fileStatus = document.getElementById("fileStatus");
    const analysisOutput = document.getElementById("analysisOutput");
    const statusText = document.getElementById("statusText");

    const aboutModal = document.getElementById("aboutModal");
    const aboutBtn = document.getElementById("aboutBtn");
    const closeAboutBtn = document.getElementById("closeAboutBtn");

    // ----------------------------------------------------
    // Logging bridge (C++ → JS → UI)
    // ----------------------------------------------------
    window.binforgeLog = (msg) => {
        analysisOutput.textContent += msg;
        analysisOutput.scrollTop = analysisOutput.scrollHeight;
    };

    // ----------------------------------------------------
    // WASM init (MODULARIZE=1 + build in wasm/build/)
    // ----------------------------------------------------
    import("./wasm/build/binforge.js").then((createModule) => {

        return createModule.default({
            locateFile: (path) => {
                // ensures binforge.wasm is found in same folder
                return "./wasm/build/" + path;
            }
        });

    }).then((Module) => {
        ModuleInstance = Module;
        console.log("WASM ready");
    });

    // ----------------------------------------------------
    // File load + immediate analysis
    // ----------------------------------------------------
    fileInput.addEventListener("change", async (e) => {

        const file = e.target.files[0];
        if (!file) return;

        const arrayBuffer = await file.arrayBuffer();

        state.file = file;
        state.binaryData = new Uint8Array(arrayBuffer);
        state.binarySize = state.binaryData.length;

        fileStatus.textContent = file.name;
        statusText.textContent =
            "Binary loaded (" + state.binarySize + " bytes)";

        analysisOutput.textContent =
            "Loaded binary:\n" +
            "Name: " + file.name + "\n" +
            "Size: " + state.binarySize + " bytes\n\n";

        // ------------------------------------------------
        // WASM analysis (guarded)
        // ------------------------------------------------
        if (!ModuleInstance) {
            analysisOutput.textContent += "WASM not ready yet...\n";
            return;
        }

        const ptr = ModuleInstance._malloc(state.binarySize);
        ModuleInstance.HEAPU8.set(state.binaryData, ptr);

        ModuleInstance.ccall(
            "analyze_esp32_binary",
            null,
            ["number", "number"],
            [ptr, state.binarySize]
        );

        ModuleInstance._free(ptr);
    });

    function openAbout() {
        aboutModal.classList.add("open");
    }

    function closeAbout() {
        aboutModal.classList.remove("open");
    }

    aboutBtn.addEventListener("click", openAbout);
    closeAboutBtn.addEventListener("click", closeAbout);

    aboutModal.addEventListener("click", (e) => {
        if (e.target === aboutModal)
            closeAbout();
    });

});