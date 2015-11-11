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

	var baseOverlays = [{"id" : "__none__", "name" : "None"}];
	var otherOverlays = [];
	for (var i in overlays) {
		var overlayConfig = this.ui.getOverlayConfig(overlays[i]);
		if (overlayConfig.base)
			baseOverlays.push(overlayConfig);
		else
			otherOverlays.push(overlayConfig);
	}

	this.control.div.innerHTML = "";
	this.checkboxes = {};

	var label = document.createElement("div");
	label.innerHTML = "<b>Base Overlays:</b>";
	this.control.div.appendChild(label);
	for (var i in baseOverlays) {
		var overlay = baseOverlays[i];

		var container = document.createElement("div");
		var radio = document.createElement("input");
		radio.setAttribute("id", "cb_overlay_" + overlay.id);
		radio.setAttribute("data-overlay", overlay.id);
		radio.setAttribute("name", "base-overlay");
		radio.setAttribute("type", "radio");
		radio.style.verticalAlign = "middle";
		radio.checked = overlay.id == "__none__";
		this.checkboxes[overlay.id] = radio;
	
		var self = this;
		radio.addEventListener("change", function() {
			var thisOverlay = this.getAttribute("data-overlay");
			self.ui.enableOverlay(this.getAttribute("data-overlay"), this.checked);
			// disable all other base overlays
			if (this.checked) {
				for (var i2 in baseOverlays) {
					if (thisOverlay != baseOverlays[i2].id)
						self.ui.enableOverlay(baseOverlays[i2].id, false);
				}
			}
		});

		var label = document.createElement("label");
		label.setAttribute("for", "cb_overlay_" + overlay.id);
		label.innerHTML = overlay.name;

		container.appendChild(radio);
		container.appendChild(label);
		this.control.div.appendChild(container);
	}

	var label = document.createElement("div");
	label.innerHTML = "<b>Additional Overlays:</b>";
	this.control.div.appendChild(label);
	for (var i in otherOverlays) {
		var overlay = otherOverlays[i];
		
		var container = document.createElement("div");
		var checkbox = document.createElement("input");
		checkbox.setAttribute("id", "cb_overlay_" + overlay.id);
		checkbox.setAttribute("data-overlay", overlay.id);
		checkbox.setAttribute("type", "checkbox");
		checkbox.style.verticalAlign = "middle";
		checkbox.checked = false;
		this.checkboxes[overlay.id] = checkbox;
	
		var self = this;
		checkbox.addEventListener("change", function() {
			self.ui.enableOverlay(this.getAttribute("data-overlay"), this.checked);
		});
		
		var label = document.createElement("label");
		label.setAttribute("for", "cb_overlay_" + overlay.id);
		label.innerHTML = overlay.name;
		
		container.appendChild(checkbox);
		container.appendChild(label);
		this.control.div.appendChild(container);
	}
};

