MapSelectControl.prototype = new BaseControl("MapSelectControl");

/**
 * This control widget allows the user to select a map.
 */
function MapSelectControl() {
	this.handler = new MapSelectHandler(this);
	this.button = null;
	this.dropdown = null;
}

MapSelectControl.prototype.create = function(wrapper) {
	var button = document.createElement("button");
	button.setAttribute("type", "button");
	button.setAttribute("class", "btn btn-default dropdown-toggle");
	button.setAttribute("data-toggle", "dropdown");
	button.setAttribute("aria-haspopup", "true");
	button.setAttribute("aria-expanded", "false");

	var ul = document.createElement("ul");
	ul.setAttribute("id", "map-ul");
	ul.setAttribute("class", "dropdown-menu dropdown-menu-right");

	this.button = button;
	this.dropdown = ul;

	for(var i in this.ui.getMapConfigsOrder()) {
		var type = this.ui.getMapConfigsOrder()[i];
		var li = document.createElement("li");
		var a = document.createElement("a");

		a.innerHTML = this.ui.getMapConfig(type).name;
		a.setAttribute("data-map", type);
		li.setAttribute("data-map", type);

		a.addEventListener("click", function(ui) {
			return function() {
				ui.setMap(this.getAttribute("data-map"));
			}
		}(this.ui));

		li.appendChild(a);
		ul.appendChild(li);
	}


	L.DomEvent.disableClickPropagation(wrapper);
	wrapper.appendChild(this.button);
	wrapper.appendChild(this.dropdown);
};

MapSelectControl.prototype.getHandler = function() {
	return this.handler;
}

MapSelectControl.prototype.getName = function() {
	return 'map-select';
};

MapSelectControl.prototype.usePanelWrapper = function() {
	return false;
};

