.PHONY: clean All

All:
	@echo "----------Building project:[ stas - Debug ]----------"
	@"$(MAKE)" -f  "stas.mk"
clean:
	@echo "----------Cleaning project:[ stas - Debug ]----------"
	@"$(MAKE)" -f  "stas.mk" clean
