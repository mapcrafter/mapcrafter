OverlaySelectHandler.prototype = new BaseHandler();

/**
 * Updates the control widget with the map rotation selection.
 */
function OverlaySelectHandler(control) {
	this.control = control
}

OverlaySelectHandler.prototype.create = function() {
};

OverlaySelectHandler.prototype.onMapChange = function(map, rotation) {
	this.update(map, rotation);
};

OverlaySelectHandler.prototype.update = function(map, rotation) {	
	var config = this.ui.getCurrentMapConfig();

	this.control.div.innerHTML = "";
	for (var i in config.overlays) {
		var overlay = config.overlays[i];
		
		var container = document.createElement("div");
		var checkbox = document.createElement("input");
		checkbox.setAttribute("id", "cb_overlay_" + overlay);
		checkbox.setAttribute("data-overlay", overlay);
		checkbox.setAttribute("type", "checkbox");
		checkbox.style.verticalAlign = "middle";
		checkbox.checked = false;
	
		var self = this;
		checkbox.addEventListener("change", function() {
			var overlay = "overlay_" + this.getAttribute("data-overlay");
			var layer = self.ui.layers[map][rotation][overlay];
			if(this.checked)
				self.ui.lmap.addLayer(layer);
			else
				self.ui.lmap.removeLayer(layer);
			// self.handler.show(group, this.checked);
		});
		
		var label = document.createElement("label");
		label.setAttribute("for", "cb_overlay_" + overlay);
		label.innerHTML = overlay;
		
		container.appendChild(checkbox);
		container.appendChild(label);
		this.control.div.appendChild(container);
	}
};
