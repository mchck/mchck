<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/TR/xhtml1/strict">

  <xsl:output method="text" />

  <xsl:template match="/Device">
    <xsl:text>/*
 * Linker script for </xsl:text>
    <xsl:value-of select="Name"/>
    <xsl:text>
 */

MEMORY
{
    </xsl:text>
    <xsl:for-each select="*[Block]">
      <xsl:text>	</xsl:text>
      <xsl:choose>
	<xsl:when test="local-name() = 'Flash'">
	  <xsl:text>rom</xsl:text>
	</xsl:when>
	<xsl:when test="local-name() = 'Ram'">
	  <xsl:text>ram</xsl:text>
	</xsl:when>
      </xsl:choose>
      <xsl:text> (</xsl:text>
      <xsl:if test="Block/@read = 'true'">r</xsl:if>
      <xsl:if test="Block/@write = 'true'">w</xsl:if>
      <xsl:if test="Block/@execute = 'true'">x</xsl:if>
      <xsl:text>) : ORIGIN = </xsl:text>
      <xsl:value-of select="Block/@addr"/>
      <xsl:text>, LENGTH = </xsl:text>
      <xsl:value-of select="Block/@size"/>
      <xsl:text>
</xsl:text>
    </xsl:for-each>
    <xsl:text>}

_eram = </xsl:text>
    <xsl:value-of select="Ram/Block/@addr"/> + <xsl:value-of select="Ram/Block/@size"/>
    <xsl:text>;
</xsl:text>
  </xsl:template>
</xsl:stylesheet>
