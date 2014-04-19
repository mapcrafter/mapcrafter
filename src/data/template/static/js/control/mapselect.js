MapSelectControl.prototype = new BaseControl("MapSelectControl");

/**
 * This control widget allows the user to select a map.
 */
function MapSelectControl() {
}

MapSelectControl.prototype.create = function(wrapper) {
	var select = document.createElement("select");
	select.setAttribute("id", "map-select");
	
	for(var type in this.ui.getConfig()) {
		var option = document.createElement("option");
		option.innerHTML = this.ui.getMapConfig(type).name;
		option.setAttribute("value", type);
		if(this.ui.getCurrentMap() == type)
			option.setAttribute("selected", true);
		select.appendChild(option);
	}
	
	select.addEventListener("change", (function(ui) {
		return function() {
			ui.setMap(select.value);
		}
	})(this.ui));
	
	var text = document.createElement("span");
	text.innerHTML = "Map type: ";
	
	wrapper.appendChild(text);
	wrapper.appendChild(select);
};
