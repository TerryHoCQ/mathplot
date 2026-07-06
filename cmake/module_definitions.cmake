#
# Define variables of module groups for use by the sebsjames/mathplot
# build process itself, and by client projects.
#

# Sets up variables the define the sebsjames/maths modules used
# building mathplot. Pass in the path to the sebsjames/maths root.
macro(setup_module_variables_for_mathplot_maths base_directory)

  # These are set assuming a submoduled maths.
  set(MPLOT_MATHS_CORE_MODULES
    ${base_directory}/sm/mathconst.cppm
    ${base_directory}/sm/constexpr_math.cppm
    ${base_directory}/sm/interval.cppm
    ${base_directory}/sm/polysolve.cppm
    ${base_directory}/sm/bessel_i0.cppm
    ${base_directory}/sm/random.cppm
    ${base_directory}/sm/vec.cppm
    ${base_directory}/sm/quaternion.cppm
    ${base_directory}/sm/mat.cppm
    ${base_directory}/sm/trait_tests.cppm
    ${base_directory}/sm/util.cppm
    ${base_directory}/sm/base64.cppm
    ${base_directory}/sm/crc32.cppm
    ${base_directory}/sm/flags.cppm
    ${base_directory}/sm/algo.cppm
    ${base_directory}/sm/geometry.cppm
    ${base_directory}/sm/geometry_polyhedra.cppm
    ${base_directory}/sm/vvec.cppm
    ${base_directory}/sm/scale.cppm
    ${base_directory}/sm/centroid.cppm
    ${base_directory}/sm/spline.cppm
    ${base_directory}/sm/winder.cppm    
  )

  # The modules required for hexgrids
  set(MPLOT_MATHS_HEXGRID_MODULES
    ${base_directory}/sm/nm_simplex.cppm
    ${base_directory}/sm/binomial.cppm
    ${base_directory}/sm/bezcoord.cppm
    ${base_directory}/sm/bezcurve.cppm
    ${base_directory}/sm/bezcurvepath.cppm
    ${base_directory}/sm/hex.cppm
    ${base_directory}/sm/hexgrid.cppm
  )

  # The modules required for cartgrids
  set(MPLOT_MATHS_CARTGRID_MODULES
    ${base_directory}/sm/nm_simplex.cppm
    ${base_directory}/sm/binomial.cppm
    ${base_directory}/sm/bezcoord.cppm
    ${base_directory}/sm/bezcurve.cppm
    ${base_directory}/sm/bezcurvepath.cppm
    ${base_directory}/sm/boxfilter.cppm
    ${base_directory}/sm/grid.cppm
    ${base_directory}/sm/rect.cppm
    ${base_directory}/sm/cartgrid.cppm
  )

  # Maths used in individual VisualModels, but not the mathplot core
  set(MPLOT_MATHS_VISUALMODEL_MODULES
    ${base_directory}/sm/histo.cppm
    ${base_directory}/sm/grid.cppm
    ${base_directory}/sm/nm_simplex.cppm
    ${base_directory}/sm/bezcoord.cppm
    ${base_directory}/sm/binomial.cppm
    ${base_directory}/sm/bezcurve.cppm
    ${base_directory}/sm/bezcurvepath.cppm
    ${base_directory}/sm/hex.cppm
    ${base_directory}/sm/hexgrid.cppm
  )
endmacro()

macro(setup_module_variables_for_mathplot base_directory)

  # Base modules for mathplot. With these (and
  # MPLOT_MATHS_CORE_MODULES) you can build helloworld
  set(MPLOT_CORE_MODULES
    ${base_directory}/mplot/keys.cppm
    ${base_directory}/mplot/version.cppm
    ${base_directory}/mplot/tools.cppm
    ${base_directory}/mplot/unicode.cppm
    ${base_directory}/mplot/loadpng.cppm
    ${base_directory}/mplot/gl/version.cppm
    ${base_directory}/mplot/gl/util_mx.cppm
    ${base_directory}/mplot/colour.cppm
    ${base_directory}/mplot/win_t.cppm
    ${base_directory}/mplot/CoordArrows.cppm
    ${base_directory}/mplot/VisualOwnable.cppm
    ${base_directory}/mplot/Visual.cppm
    ${base_directory}/mplot/VisualGlfw.cppm
    ${base_directory}/mplot/VisualFace.cppm
    ${base_directory}/mplot/TextFeatures.cppm
    ${base_directory}/mplot/TextGeometry.cppm
    ${base_directory}/mplot/VisualResources.cppm
    ${base_directory}/mplot/VisualCommon.cppm
    ${base_directory}/mplot/VisualFont.cppm
    ${base_directory}/mplot/VisualTextModel.cppm
    ${base_directory}/mplot/VisualModel.cppm
    ${base_directory}/mplot/NavMesh.cppm
    ${base_directory}/mplot/ColourMap.cppm
    ${base_directory}/mplot/colourmaps_cet.cppm
    ${base_directory}/mplot/colourmaps_crameri.cppm
    ${base_directory}/mplot/ColourMap_Lists.cppm
    ${base_directory}/mplot/graphing.cppm
    ${base_directory}/mplot/graphstyles.cppm
    ${base_directory}/mplot/DatasetStyle.cppm
    ${base_directory}/mplot/VisualDataModel.cppm
    ${base_directory}/mplot/compoundray/Ommatidium.cppm
    ${base_directory}/mplot/jcvoronoi/jc_voronoi.cppm
    ${base_directory}/mplot/healpix/healpix_bare.cppm        
    ${base_directory}/mplot/fps/profiler.cppm
  )

  # All the VisualModel modules
  set(MPLOT_VISUALMODEL_MODULES
    ${base_directory}/mplot/GraphVisual.cppm    
    ${base_directory}/mplot/GridVisual.cppm
    ${base_directory}/mplot/RodVisual.cppm
    ${base_directory}/mplot/RhomboVisual.cppm
    ${base_directory}/mplot/VectorVisual.cppm
    ${base_directory}/mplot/SphereVisual.cppm
    ${base_directory}/mplot/ConeVisual.cppm
    ${base_directory}/mplot/QuiverVisual.cppm
    ${base_directory}/mplot/NormalsVisual.cppm
    ${base_directory}/mplot/GeodesicVisual.cppm
    ${base_directory}/mplot/compoundray/EyeVisual.cppm
    ${base_directory}/mplot/InstancedScatterVisual.cppm
    ${base_directory}/mplot/HexGridVisual.cppm
    ${base_directory}/mplot/CartGridVisual.cppm
    ${base_directory}/mplot/TriaxesVisual.cppm
    ${base_directory}/mplot/TxtVisual.cppm    
    ${base_directory}/mplot/ScatterVisual.cppm
    ${base_directory}/mplot/CurvyTellyVisual.cppm
    ${base_directory}/mplot/BoolVisual.cppm    
    ${base_directory}/mplot/RectangleVisual.cppm
    ${base_directory}/mplot/TriangleVisual.cppm
    ${base_directory}/mplot/VoronoiVisual.cppm
    ${base_directory}/mplot/IcosaVisual.cppm
    ${base_directory}/mplot/GratingVisual.cppm
    ${base_directory}/mplot/ColourBarVisual.cppm
    ${base_directory}/mplot/HSVWheelVisual.cppm
    ${base_directory}/mplot/CyclicColourVisual.cppm
    ${base_directory}/mplot/QuadsVisual.cppm
    ${base_directory}/mplot/QuadsMeshVisual.cppm    
    ${base_directory}/mplot/PointRowsVisual.cppm
    ${base_directory}/mplot/PointRowsMeshVisual.cppm
    ${base_directory}/mplot/RingVisual.cppm
    ${base_directory}/mplot/SphericalProjectionVisual.cppm
    ${base_directory}/mplot/HealpixVisual.cppm        
    ${base_directory}/mplot/PolygonVisual.cppm
    ${base_directory}/mplot/TriFrameVisual.cppm
    ${base_directory}/mplot/PolarVisual.cppm
)

endmacro()
