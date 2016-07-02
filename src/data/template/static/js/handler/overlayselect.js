OverlaySelectHandler.prototype = new BaseHandler();

/**
 * Updates the control widget with the map rotation selection.
 */
function OverlaySelectHandler(control) {
	this.control = control;

	this.baseOverlayButtons = {};
	this.otherOverlayButtons = {};
}

OverlaySelectHandler.prototype.create = function() {
};

OverlaySelectHandler.prototype.onMapChange = function(map, rotation) {
	this.update(map, rotation);
};

function checkButton(button, checked) {
	var checkedClass = "list-group-item-info";
	if (checked) {
		Util.addClass(button, checkedClass);
	} else {
		Util.removeClass(button, checkedClass);
	}

}

OverlaySelectHandler.prototype.onOverlayChange = function(overlay, enabled) {
	var button;
	if (!(overlay in this.baseOverlayButtons)) {
		button = this.otherOverlayButtons[overlay];
	} else {
		button = this.baseOverlayButtons[overlay];
		if (enabled ) {
			// disable other buttons if this is a base overlay button
			for (var otherOverlay in this.baseOverlayButtons) {
				checkButton(this.baseOverlayButtons[otherOverlay], false);
			}
		}
	}
	checkButton(button, enabled);
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

	this.baseOverlayButtons = {};
	this.otherOverlayButtons = {};

	var checkedClass = "list-group-item-info";
	var baseListGroup = document.createElement("div");
	baseListGroup.setAttribute("class", "list-group");
	for (var i in baseOverlays) {
		var overlay = baseOverlays[i];
		
		var button = document.createElement("button");
		button.setAttribute("type", "button");
		button.setAttribute("class", "list-group-item");
		if (overlay.id == "__none__")
			button.setAttribute("class", "list-group-item " + checkedClass);
		button.setAttribute("data-overlay", overlay.id);
		button.innerHTML = overlay.name;
		button.addEventListener("click", function(ui) {
			return function() {
				var thisOverlay = this.getAttribute("data-overlay");
				ui.enableOverlay(thisOverlay, true);
				Util.addClass(this, checkedClass);
	
				// disable all other overlays
				var buttons = baseListGroup.childNodes;
				for (var i = 0; i < buttons.length; i++) {
					var thatOverlay = buttons[i].getAttribute("data-overlay");
					if (thatOverlay != thisOverlay) {
						ui.enableOverlay(thatOverlay, false);
						Util.removeClass(buttons[i], checkedClass);
					}
				}
			}
		}(this.ui));

		baseListGroup.appendChild(button);
		this.baseOverlayButtons[overlay.id] = button;
	}

	var otherListGroup = document.createElement("div");
	otherListGroup.setAttribute("class", "list-group");
	for (var i in otherOverlays) {
		var overlay = otherOverlays[i];
		
		var button = document.createElement("button");
		button.setAttribute("type", "button");
		button.setAttribute("class", "list-group-item");
		button.setAttribute("data-overlay", overlay.id);
		button.innerHTML = overlay.name;
		button.addEventListener("click", function(ui) {
			return function() {
				var thisOverlay = this.getAttribute("data-overlay");
				var checked = Util.hasClass(this, checkedClass);
				ui.enableOverlay(thisOverlay, !checked);
	
				if (checked) {
					Util.removeClass(this, checkedClass);
				} else {
					Util.addClass(this, checkedClass);
				}
			}
		}(this.ui));

		otherListGroup.appendChild(button);
		this.otherOverlayButtons[overlay.id] = button;
	}

	var wrapper = this.control.div;
	wrapper.innerHTML = "";
	wrapper.appendChild(Util.createPanelHeader("Base Overlays"));
	wrapper.appendChild(baseListGroup);
	wrapper.appendChild(Util.createPanelHeader("Additional Overlays"));
	wrapper.appendChild(otherListGroup);
};

