MousePosControl.prototype = new BaseControl("MousePosControl");

/**
 * This control widget shows the current position of the mouse in Minecraft.
 */
function MousePosControl() {
}

MousePosControl.prototype.create = function(wrapper) {
	var text = document.createElement("span");
	text.setAttribute("id", "mouse-move-div");
	
	var updatePos = function(ui) {
		return function(event) {
			var xzy = ui.latLngToMC(event.latlng, 64);
			document.getElementById("mouse-move-div").innerHTML = "X: " + Math.round(xzy[0]) 
				+ " Z: " + Math.round(xzy[1]) + " Y: " + Math.round(xzy[2]);
		};
	}(this.ui);
	
	this.ui.lmap.on("mousemove", updatePos);
	this.ui.lmap.on("mousedown", updatePos);
	
	wrapper.appendChild(text);
}
