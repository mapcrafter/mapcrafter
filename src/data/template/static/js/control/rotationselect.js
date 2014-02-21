RotationSelectControl.prototype = new BaseControl("RotationSelectControl");

/**
 * This control widget allows the user to rotate the map.
 */
function RotationSelectControl() {
	this.handler = new RotationSelectHandler(this);
	
	this.images = [];
}

RotationSelectControl.prototype.create = function(wrapper) {
	var names = ["tl", "tr", "br", "bl"];
	for(var i = 0; i < 4; i++) {
		var a = document.createElement("a");
		a.setAttribute("href", "#");
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
		img.setAttribute("class", "rotation");
		img.setAttribute("src", "static/img/" + names[i] + ".png");
		a.appendChild(img);
		
		var img2 = document.createElement("img");
		img2.setAttribute("id", "rotation-" + names[i] + "-active");
		img2.setAttribute("class", "rotation");
		img2.setAttribute("src", "static/img/" + names[i] + "_active.png");
		
		wrapper.appendChild(a);
		wrapper.appendChild(img2);
		
		this.images.push(a);
		this.images.push(img2);
	}
	
	this.ui.addHandler(this.handler);
};
