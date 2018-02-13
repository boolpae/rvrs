.PHONY: clean All

All:
	@echo "----------Building project:[ rvrs - Debug ]----------"
	@"$(MAKE)" -f  "rvrs.mk"
clean:
	@echo "----------Cleaning project:[ rvrs - Debug ]----------"
	@"$(MAKE)" -f  "rvrs.mk" clean
