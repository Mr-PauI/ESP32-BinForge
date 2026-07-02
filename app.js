document.addEventListener("DOMContentLoaded", () => {

    let state = {
        file: null,
        binaryData: null,
        binarySize: 0
    };

    // ----------------------------------------------------
    // WASM state (ONLY ONCE — FIXED)
    // ----------------------------------------------------
    let ModuleInstance = null;
    let wasmReady = false;

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
    // WASM init (MODULARIZE=1 + correct Emscripten style)
    // ----------------------------------------------------
    function initWASM() {

        if (typeof BinForgeWASM !== "function") {
            console.error("BinForgeWASM not available yet");
            return;
        }

        BinForgeWASM({
            locateFile: (path) =>
                new URL("./wasm/" + path, document.baseURI).href,

            onRuntimeInitialized: function () {
                ModuleInstance = this;
                wasmReady = true;

                console.log("WASM ready");

                statusText.textContent = "Waiting for firmware...";
            }
        });
    }

    window.addEventListener("load", initWASM);

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
        // WASM analysis (SAFE GUARD)
        // ------------------------------------------------
        if (!wasmReady || !ModuleInstance) {
            analysisOutput.textContent +=
                "WASM not ready yet — try again in a second.\n";
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

    // ----------------------------------------------------
    // About modal
    // ----------------------------------------------------
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