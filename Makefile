include $(TOPDIR)/rules.mk

PKG_NAME:=zk-ubustest
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/zk-ubustest
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libubus +libubox +libblobmsg-json
  TITLE:=ZK ubus test
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


#TARGET_CFLAGS += -ldl
TARGET_LDFLAGS += -lubus -lubox -lblobmsg_json

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef


define Package/zk-ubustest/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ubustest $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,zk-ubustest))
