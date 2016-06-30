RotationSelectControl.prototype = new BaseControl("RotationSelectControl");

/**
 * This control widget allows the user to rotate the map.
 */
function RotationSelectControl() {
	this.handler = new RotationSelectHandler(this);
	
	this.images = [];
}

RotationSelectControl.prototype.create = function(wrapper) {
	var buttons = document.createElement("div");
	buttons.setAttribute("class", "btn-group");
	buttons.setAttribute("role", "group");

	var names = ["tl", "tr", "br", "bl"];
	for(var i = 0; i < 4; i++) {
		var a = document.createElement("button");
		a.setAttribute("class", "btn btn-default");
		a.setAttribute("data-rotation", i);
		a.addEventListener("click", (function(ui) {
			return function(event) {
				event.preventDefault();
				ui.setMapRotation(this.getAttribute("data-rotation"));
				return false;
			};
		})(this.ui));
		
		img = document.createElement("img");
		img.setAttribute("id", "rotation-" + names[i]);
		img.setAttribute("src", "static/img/" + names[i] + "_active.png");
		
		var img2 = document.createElement("img");
		img2.setAttribute("id", "rotation-" + names[i] + "-active");
		img2.setAttribute("src", "static/img/" + names[i] + ".png");
		
		a.appendChild(img);
		a.appendChild(img2);
		buttons.appendChild(a);
		
		this.images.push(img);
		this.images.push(img2);
	}

	wrapper.appendChild(buttons);
	
	this.ui.addHandler(this.handler);
};

RotationSelectControl.prototype.getName = function() {
	return "rotation-select";
};

RotationSelectControl.prototype.usePanelWrapper = function() {
	return false;
};

