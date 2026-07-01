document.addEventListener("DOMContentLoaded", () => {

    let state = { file: null };

    const fileInput = document.getElementById("fileInput");
    const fileStatus = document.getElementById("fileStatus");
    const analysisOutput = document.getElementById("analysisOutput");
    const statusText = document.getElementById("statusText");

    const aboutModal = document.getElementById("aboutModal");
    const aboutBtn = document.getElementById("aboutBtn");
    const closeAboutBtn = document.getElementById("closeAboutBtn");

    fileInput.addEventListener("change", (e) => {
        const file = e.target.files[0];
        if (!file) return;

        state.file = file;

        fileStatus.textContent = file.name;
        statusText.textContent = "File loaded (no analysis yet)";

        analysisOutput.textContent =
            "Loaded file:\n" +
            "Name: " + file.name + "\n" +
            "Size: " + file.size + " bytes\n\n" +
            "WASM not yet connected.";
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
        if (e.target === aboutModal) closeAbout();
    });

});