MapSelectControl.prototype = new BaseControl("MapSelectControl");

/**
 * This control widget allows the user to select a map.
 */
function MapSelectControl() {
	this.handler = new MapSelectHandler(this);
	this.button = null;
}

MapSelectControl.prototype.create = function(wrapper) {
	var button = document.createElement("button");
	button.setAttribute("type", "button");
	button.setAttribute("class", "btn btn-default dropdown-toggle");
	button.setAttribute("data-toggle", "dropdown");
	button.setAttribute("aria-haspopup", "true");
	button.setAttribute("aria-expanded", "false");
	wrapper.appendChild(button);
	this.button = button;

	var select = document.createElement("ul");
	select.setAttribute("id", "map-select");
	select.setAttribute("class", "dropdown-menu dropdown-menu-right");

	for(var i in this.ui.getMapConfigsOrder()) {
		var type = this.ui.getMapConfigsOrder()[i];
		var option = document.createElement("li");
		var link = document.createElement("a");

		link.innerHTML = this.ui.getMapConfig(type).name;
		link.setAttribute("data-map", type);

		link.addEventListener("click", function(ui) {
			return function(a) {
				ui.setMap(a.target.getAttribute("data-map"));
			}
		}(this.ui));

		option.appendChild(link);
		select.appendChild(option);
	}

	L.DomEvent.disableClickPropagation(wrapper);
	wrapper.appendChild(select);
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

