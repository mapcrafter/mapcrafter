RotationSelectControl.prototype = new BaseControl("RotationSelectControl");

/**
 * This control widget allows the user to rotate the map.
 */
function RotationSelectControl() {
	this.handler = new RotationSelectHandler(this);
	
	this.buttons = [];
}

RotationSelectControl.prototype.create = function(wrapper) {
	var buttonGroup = document.createElement("div");
	buttonGroup.setAttribute("class", "btn-group");
	buttonGroup.setAttribute("role", "group");

	var names = ["tl", "tr", "br", "bl"];
	for(var i = 0; i < 4; i++) {
		var button = document.createElement("button");
		button.setAttribute("class", "btn btn-default");
		button.setAttribute("data-rotation", i);
		button.addEventListener("click", (function(ui) {
			return function(event) {
				ui.setMapRotation(this.getAttribute("data-rotation"));
			};
		})(this.ui));
		
		img = document.createElement("img");
		img.setAttribute("id", "rotation-" + names[i]);
		img.setAttribute("src", "static/img/" + names[i] + ".png");
		button.appendChild(img);

		buttonGroup.appendChild(button);
		this.buttons.push(button);
	}

	wrapper.appendChild(buttonGroup);
};

RotationSelectControl.prototype.getHandler = function() {
	return this.handler;
};

RotationSelectControl.prototype.getName = function() {
	return "rotation-select";
};

RotationSelectControl.prototype.usePanelWrapper = function() {
	return false;
};

