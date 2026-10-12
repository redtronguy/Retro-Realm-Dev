# Saltmere harbour — alternate camera references

The user requested left, right, rear and top-down views of the [approved base](../saltmere-harbour.png) to help Claude reconstruct the map in Unreal. Every final generation received that actual base PNG as its visual reference. These images supplement the base; they do not replace it.

| View | Image | What to inspect |
|---|---|---|
| Left / ship side | [Left view](saltmere-left.png) | Gangplank, quay edge, ship clearance and terrace elevations. |
| Right / lighthouse side | [Right view](saltmere-right.png) | Harbour-wall walkway, lighthouse landing and its connection to the quay. |
| Rear / town looking seaward | [Rear view](saltmere-rear.png) | Descending town stairs, quay length, skyline and reverse-view occlusion. The ship appears screen-right and lighthouse screen-left. |
| Top-down / overhead | [Top-down view](saltmere-top-down.png) | Main quay footprint, stair runs, gangplank and secondary harbour-wall route. Ship sails are furled to expose the deck. |

[prompts.json](prompts.json) records the exact final prompt for each view. All four PNGs retain their generated 1536×1024 resolution without resizing or image edits. Earlier rejected overhead attempts are not included.

User follow-up: [wider overhead view](saltmere-top-down-zoom-out-50.png), generated with the top-down PNG attached and a requested 50% reduction in linear scene scale. [Exact zoom-out prompt](top-down-zoom-out-prompt.txt). It reveals additional town approaches and shoreline; the expanded surroundings are inferred, and zoom scale is approximate.

## Shared layout anchors

For orientation, north means inland/uphill and south means the foreground harbour approach in the approved base. These are reconstruction coordinates, not a claim about the novel's geographic compass.

- Main movement route: south arrival/market → central quay junction → broad north stair → upper-town archway.
- Ship branch: central quay junction → west gangplank → three-masted *Gull's Promise*.
- Lighthouse branch: central quay junction → east harbour wall → lighthouse landing.
- Houses occupy the north terraces; stalls and cargo sit beside the movement routes.
- Left/right describe viewpoints from the approved base's ship/lighthouse sides. The generated angles are approximate, not calibrated 90-degree camera orbits.

## Reconcile before building

These are independently generated concept views, not renders from a shared 3D model, photogrammetry, measured plans or collision-verified levels. Hidden architecture is inferred. Props, building details, ship scale and exact quay/wall proportions can drift between views; the overhead image also retains some stylized depth. Keep the approved base authoritative for appearance and use these views to understand connections and elevation intent.

Create one consistent code-authored blockout from those anchors, choose actual dimensions using the playable hero, and render these four camera positions from that same Unreal blockout. Verify every route in motion before detailing it. Do not copy conflicting generated geometry into separate pieces of the level.
