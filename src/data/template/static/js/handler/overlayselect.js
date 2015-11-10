OverlaySelectHandler.prototype = new BaseHandler();

/**
 * Updates the control widget with the map rotation selection.
 */
function OverlaySelectHandler(control) {
	this.control = control;

	this.checkboxes = {};
}

OverlaySelectHandler.prototype.create = function() {
};

OverlaySelectHandler.prototype.onMapChange = function(map, rotation) {
	this.update(map, rotation);
};

OverlaySelectHandler.prototype.onOverlayChange = function(overlay, enabled) {
	this.checkboxes[overlay].checked = enabled;
}

OverlaySelectHandler.prototype.update = function(map, rotation) {	
	var overlays = this.ui.getCurrentOverlays();

	this.checkboxes = {};
	this.control.div.innerHTML = "";
	for (var i in overlays) {
		var overlay = overlays[i];
		var overlayConfig = this.ui.getOverlayConfig(overlays[i]);
		
		var container = document.createElement("div");
		var checkbox = document.createElement("input");
		checkbox.setAttribute("id", "cb_overlay_" + overlayConfig.name);
		checkbox.setAttribute("data-overlay", overlay);
		checkbox.setAttribute("type", "checkbox");
		checkbox.style.verticalAlign = "middle";
		checkbox.checked = false;
		this.checkboxes[overlay] = checkbox;
	
		var self = this;
		checkbox.addEventListener("change", function() {
			self.ui.enableOverlay(this.getAttribute("data-overlay"), this.checked);
		});
		
		var label = document.createElement("label");
		label.setAttribute("for", "cb_overlay_" + overlay);
		label.innerHTML = overlayConfig.name;
		
		container.appendChild(checkbox);
		container.appendChild(label);
		this.control.div.appendChild(container);
	}
};

