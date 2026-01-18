"""
Solution Visualization for the Routing Library.

This module provides functions to visualize VRP solutions using matplotlib.

Example:
    >>> import routing
    >>> from routing.visualization import plot_solution
    >>>
    >>> routing.init()
    >>> problem = routing.load_solomon("data/c101.txt")
    >>> solution = routing.solve(problem, "ga", timeout=30)
    >>> plot_solution(problem, solution)
"""

from typing import Optional, List, Tuple, Dict, Any
import math

# Color palette for routes (colorblind-friendly)
DEFAULT_COLORS = [
    '#1f77b4',  # blue
    '#ff7f0e',  # orange
    '#2ca02c',  # green
    '#d62728',  # red
    '#9467bd',  # purple
    '#8c564b',  # brown
    '#e377c2',  # pink
    '#7f7f7f',  # gray
    '#bcbd22',  # olive
    '#17becf',  # cyan
]


def plot_solution(
    problem,
    solution,
    ax=None,
    figsize: Tuple[int, int] = (10, 8),
    title: Optional[str] = None,
    show_labels: bool = True,
    show_demand: bool = False,
    show_legend: bool = True,
    depot_marker: str = 's',
    depot_size: int = 200,
    depot_color: str = 'black',
    client_marker: str = 'o',
    client_size: int = 100,
    unserved_color: str = 'red',
    route_colors: Optional[List[str]] = None,
    route_width: float = 1.5,
    route_alpha: float = 0.7,
    arrow_size: int = 15,
    show_arrows: bool = True,
    font_size: int = 8,
    save_path: Optional[str] = None,
    dpi: int = 150,
):
    """
    Plot a VRP solution showing routes on a 2D plane.

    Args:
        problem: The routing Problem object
        solution: The Solution object to visualize
        ax: Optional matplotlib axes. If None, creates new figure
        figsize: Figure size as (width, height) tuple
        title: Plot title. If None, auto-generates based on solution cost
        show_labels: Show client ID labels
        show_demand: Show demand values next to clients
        show_legend: Show route legend
        depot_marker: Marker style for depot ('s'=square, 'o'=circle, etc.)
        depot_size: Marker size for depot
        depot_color: Color for depot marker
        client_marker: Marker style for clients
        client_size: Marker size for clients
        unserved_color: Color for unserved clients
        route_colors: List of colors for routes. Uses default palette if None
        route_width: Line width for routes
        route_alpha: Transparency for route lines (0-1)
        arrow_size: Size of direction arrows
        show_arrows: Whether to show direction arrows on routes
        font_size: Font size for labels
        save_path: If provided, saves the figure to this path
        dpi: DPI for saved figure

    Returns:
        matplotlib Figure and Axes objects (fig, ax)

    Example:
        >>> fig, ax = plot_solution(problem, solution)
        >>> plt.show()

        >>> # Save to file
        >>> plot_solution(problem, solution, save_path="route.png")

        >>> # Customize appearance
        >>> plot_solution(problem, solution,
        ...               show_demand=True,
        ...               route_width=2.0,
        ...               title="My Solution")
    """
    try:
        import matplotlib.pyplot as plt
        from matplotlib.lines import Line2D
    except ImportError:
        raise ImportError(
            "matplotlib is required for visualization. "
            "Install it with: pip install matplotlib"
        )

    # Create figure if needed
    if ax is None:
        fig, ax = plt.subplots(figsize=figsize)
    else:
        fig = ax.get_figure()

    # Get colors
    colors = route_colors or DEFAULT_COLORS

    # Get depot location
    depots = problem.get_depots()
    depot_x, depot_y = None, None
    if depots:
        depot = depots[0]
        depot_x = depot.x
        depot_y = depot.y

    # Build client location map
    client_locations = {}
    client_demands = {}
    for client in problem.get_clients():
        client_id = client.get_id()
        if client.x is not None and client.y is not None:
            client_locations[client_id] = (client.x, client.y)
        if client.demand is not None:
            client_demands[client_id] = client.demand

    # Get unserved clients
    unserved_ids = set(solution.unserved) if solution else set()

    # Plot routes
    tours = solution.get_tours() if solution else []
    legend_elements = []

    for tour_idx, tour in enumerate(tours):
        client_ids = tour.get_client_ids()
        if not client_ids:
            continue

        color = colors[tour_idx % len(colors)]

        # Build route path: depot -> clients -> depot
        route_x = []
        route_y = []

        # Start from depot
        if depot_x is not None:
            route_x.append(depot_x)
            route_y.append(depot_y)

        # Add clients
        for cid in client_ids:
            if cid in client_locations:
                x, y = client_locations[cid]
                route_x.append(x)
                route_y.append(y)

        # Return to depot
        if depot_x is not None and len(route_x) > 1:
            route_x.append(depot_x)
            route_y.append(depot_y)

        # Plot route line
        ax.plot(route_x, route_y, '-', color=color,
                linewidth=route_width, alpha=route_alpha, zorder=1)

        # Plot direction arrows
        if show_arrows and len(route_x) > 1:
            for i in range(len(route_x) - 1):
                mid_x = (route_x[i] + route_x[i + 1]) / 2
                mid_y = (route_y[i] + route_y[i + 1]) / 2
                dx = route_x[i + 1] - route_x[i]
                dy = route_y[i + 1] - route_y[i]
                if dx != 0 or dy != 0:
                    ax.annotate('', xy=(mid_x + dx * 0.01, mid_y + dy * 0.01),
                                xytext=(mid_x, mid_y),
                                arrowprops=dict(arrowstyle='->', color=color,
                                                lw=route_width * 0.8),
                                zorder=2)

        # Plot clients in this tour
        for cid in client_ids:
            if cid in client_locations:
                x, y = client_locations[cid]
                ax.scatter(x, y, c=color, s=client_size, marker=client_marker,
                           edgecolors='white', linewidths=0.5, zorder=3)

        # Add to legend
        tour_cost = tour.cost
        tour_demand = sum(client_demands.get(cid, 0) for cid in client_ids)
        label = f"Route {tour_idx + 1}: {len(client_ids)} clients"
        if tour_demand > 0:
            label += f", demand={tour_demand}"
        legend_elements.append(
            Line2D([0], [0], color=color, linewidth=route_width,
                   label=label)
        )

    # Plot unserved clients
    if unserved_ids:
        for cid in unserved_ids:
            if cid in client_locations:
                x, y = client_locations[cid]
                ax.scatter(x, y, c=unserved_color, s=client_size,
                           marker='x', linewidths=2, zorder=4)
        legend_elements.append(
            Line2D([0], [0], color=unserved_color, marker='x', linestyle='',
                   markersize=10, label=f"Unserved ({len(unserved_ids)})")
        )

    # Plot depot
    if depot_x is not None:
        ax.scatter(depot_x, depot_y, c=depot_color, s=depot_size,
                   marker=depot_marker, edgecolors='white', linewidths=1,
                   zorder=5, label='Depot')
        legend_elements.insert(0,
            Line2D([0], [0], color=depot_color, marker=depot_marker,
                   linestyle='', markersize=10, label='Depot')
        )

    # Add labels
    if show_labels:
        # Depot label
        if depot_x is not None:
            ax.annotate('D', (depot_x, depot_y),
                        xytext=(5, 5), textcoords='offset points',
                        fontsize=font_size, fontweight='bold')

        # Client labels
        for cid, (x, y) in client_locations.items():
            label = str(cid)
            if show_demand and cid in client_demands:
                label += f"\n({client_demands[cid]})"
            ax.annotate(label, (x, y),
                        xytext=(5, 5), textcoords='offset points',
                        fontsize=font_size)

    # Set title
    if title is None:
        cost = solution.cost if solution else 0
        num_tours = len(tours)
        title = f"Solution: {num_tours} routes, cost={cost:.2f}"
        if unserved_ids:
            title += f" ({len(unserved_ids)} unserved)"
    ax.set_title(title, fontsize=12, fontweight='bold')

    # Labels and grid
    ax.set_xlabel('X', fontsize=10)
    ax.set_ylabel('Y', fontsize=10)
    ax.grid(True, alpha=0.3)

    # Legend
    if show_legend and legend_elements:
        ax.legend(handles=legend_elements, loc='upper left',
                  bbox_to_anchor=(1.02, 1), fontsize=font_size)
        fig.tight_layout()

    # Save if requested
    if save_path:
        fig.savefig(save_path, dpi=dpi, bbox_inches='tight')

    return fig, ax


def plot_problem(
    problem,
    ax=None,
    figsize: Tuple[int, int] = (10, 8),
    title: Optional[str] = None,
    show_labels: bool = True,
    show_demand: bool = True,
    depot_marker: str = 's',
    depot_size: int = 200,
    depot_color: str = 'black',
    client_marker: str = 'o',
    client_size: int = 100,
    client_color: str = '#1f77b4',
    font_size: int = 8,
    save_path: Optional[str] = None,
    dpi: int = 150,
):
    """
    Plot a VRP problem showing depot and client locations.

    Args:
        problem: The routing Problem object
        ax: Optional matplotlib axes
        figsize: Figure size
        title: Plot title
        show_labels: Show client ID labels
        show_demand: Show demand values
        depot_marker: Marker style for depot
        depot_size: Marker size for depot
        depot_color: Color for depot
        client_marker: Marker style for clients
        client_size: Marker size for clients
        client_color: Color for clients
        font_size: Font size for labels
        save_path: Path to save figure
        dpi: DPI for saved figure

    Returns:
        matplotlib Figure and Axes objects (fig, ax)

    Example:
        >>> fig, ax = plot_problem(problem)
        >>> plt.show()
    """
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        raise ImportError(
            "matplotlib is required for visualization. "
            "Install it with: pip install matplotlib"
        )

    if ax is None:
        fig, ax = plt.subplots(figsize=figsize)
    else:
        fig = ax.get_figure()

    # Plot depot
    depots = problem.get_depots()
    if depots:
        depot = depots[0]
        if depot.x is not None and depot.y is not None:
            ax.scatter(depot.x, depot.y, c=depot_color, s=depot_size,
                       marker=depot_marker, edgecolors='white', linewidths=1,
                       zorder=3, label='Depot')
            if show_labels:
                ax.annotate('D', (depot.x, depot.y),
                            xytext=(5, 5), textcoords='offset points',
                            fontsize=font_size, fontweight='bold')

    # Plot clients
    for client in problem.get_clients():
        if client.x is None or client.y is None:
            continue
        ax.scatter(client.x, client.y, c=client_color, s=client_size,
                   marker=client_marker, edgecolors='white', linewidths=0.5,
                   zorder=2)

        if show_labels:
            label = str(client.get_id())
            if show_demand and client.demand is not None:
                label += f"\n({client.demand})"
            ax.annotate(label, (client.x, client.y),
                        xytext=(5, 5), textcoords='offset points',
                        fontsize=font_size)

    # Set title
    if title is None:
        title = f"Problem: {problem.num_clients} clients, {problem.num_vehicles} vehicles"
        if problem.total_demand > 0:
            title += f", total demand={problem.total_demand}"
    ax.set_title(title, fontsize=12, fontweight='bold')

    ax.set_xlabel('X', fontsize=10)
    ax.set_ylabel('Y', fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right', fontsize=font_size)

    if save_path:
        fig.savefig(save_path, dpi=dpi, bbox_inches='tight')

    return fig, ax


def plot_convergence(
    costs: List[float],
    ax=None,
    figsize: Tuple[int, int] = (10, 6),
    title: str = "Convergence",
    xlabel: str = "Iteration",
    ylabel: str = "Cost",
    color: str = '#1f77b4',
    linewidth: float = 1.5,
    show_best: bool = True,
    save_path: Optional[str] = None,
    dpi: int = 150,
):
    """
    Plot convergence curve from solver callback data.

    Args:
        costs: List of solution costs collected during solving
        ax: Optional matplotlib axes
        figsize: Figure size
        title: Plot title
        xlabel: X-axis label
        ylabel: Y-axis label
        color: Line color
        linewidth: Line width
        show_best: Show horizontal line at best cost
        save_path: Path to save figure
        dpi: DPI for saved figure

    Returns:
        matplotlib Figure and Axes objects (fig, ax)

    Example:
        >>> costs = []
        >>> def callback(solution, cost):
        ...     costs.append(cost)
        >>> solution = routing.solve_with_callback(problem, callback, "ga", 30)
        >>> plot_convergence(costs)
    """
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        raise ImportError(
            "matplotlib is required for visualization. "
            "Install it with: pip install matplotlib"
        )

    if ax is None:
        fig, ax = plt.subplots(figsize=figsize)
    else:
        fig = ax.get_figure()

    iterations = list(range(1, len(costs) + 1))
    ax.plot(iterations, costs, '-', color=color, linewidth=linewidth)

    if show_best and costs:
        best = min(costs)
        ax.axhline(y=best, color='green', linestyle='--', alpha=0.7,
                   label=f'Best: {best:.2f}')
        ax.legend(loc='upper right')

    ax.set_title(title, fontsize=12, fontweight='bold')
    ax.set_xlabel(xlabel, fontsize=10)
    ax.set_ylabel(ylabel, fontsize=10)
    ax.grid(True, alpha=0.3)

    if save_path:
        fig.savefig(save_path, dpi=dpi, bbox_inches='tight')

    return fig, ax


def create_animation(
    problem,
    solutions: List,
    interval: int = 500,
    figsize: Tuple[int, int] = (10, 8),
    save_path: Optional[str] = None,
    **plot_kwargs
):
    """
    Create an animation showing solution evolution.

    Args:
        problem: The routing Problem object
        solutions: List of Solution objects (e.g., from callback)
        interval: Time between frames in milliseconds
        figsize: Figure size
        save_path: Path to save animation (requires ffmpeg for .mp4)
        **plot_kwargs: Additional arguments passed to plot_solution

    Returns:
        matplotlib FuncAnimation object

    Example:
        >>> solutions = []
        >>> def callback(solution, cost):
        ...     solutions.append(solution.clone())
        >>> routing.solve_with_callback(problem, callback, "ga", 30)
        >>> ani = create_animation(problem, solutions)
        >>> plt.show()
    """
    try:
        import matplotlib.pyplot as plt
        from matplotlib.animation import FuncAnimation
    except ImportError:
        raise ImportError(
            "matplotlib is required for visualization. "
            "Install it with: pip install matplotlib"
        )

    fig, ax = plt.subplots(figsize=figsize)

    def update(frame):
        ax.clear()
        solution = solutions[frame]
        plot_solution(problem, solution, ax=ax, show_legend=False, **plot_kwargs)
        ax.set_title(f"Iteration {frame + 1}/{len(solutions)}, Cost: {solution.cost:.2f}")
        return ax,

    ani = FuncAnimation(fig, update, frames=len(solutions),
                        interval=interval, blit=False, repeat=True)

    if save_path:
        ani.save(save_path, writer='ffmpeg', dpi=150)

    return ani


def compare_solutions(
    problem,
    solutions: Dict[str, Any],
    figsize: Tuple[int, int] = (15, 5),
    save_path: Optional[str] = None,
    **plot_kwargs
):
    """
    Compare multiple solutions side by side.

    Args:
        problem: The routing Problem object
        solutions: Dictionary mapping names to Solution objects
        figsize: Figure size
        save_path: Path to save figure
        **plot_kwargs: Additional arguments passed to plot_solution

    Returns:
        matplotlib Figure and list of Axes

    Example:
        >>> solutions = {
        ...     "GA": routing.solve(problem, "ga", timeout=30),
        ...     "VNS": routing.solve(problem, "vns", timeout=30),
        ...     "ALNS": routing.solve(problem, "alns", timeout=30),
        ... }
        >>> compare_solutions(problem, solutions)
    """
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        raise ImportError(
            "matplotlib is required for visualization. "
            "Install it with: pip install matplotlib"
        )

    n = len(solutions)
    fig, axes = plt.subplots(1, n, figsize=figsize)
    if n == 1:
        axes = [axes]

    for ax, (name, solution) in zip(axes, solutions.items()):
        if solution is not None:
            cost = solution.cost
            title = f"{name}\nCost: {cost:.2f}"
        else:
            title = f"{name}\nNo solution"
        plot_solution(problem, solution, ax=ax, title=title,
                      show_legend=False, **plot_kwargs)

    fig.tight_layout()

    if save_path:
        fig.savefig(save_path, dpi=150, bbox_inches='tight')

    return fig, axes


def plot_gantt(
    problem,
    solution,
    ax=None,
    figsize: Tuple[int, int] = (14, 6),
    title: Optional[str] = None,
    route_colors: Optional[List[str]] = None,
    tw_color: str = '#e0e0e0',
    tw_alpha: float = 0.5,
    service_alpha: float = 0.9,
    travel_color: str = '#cccccc',
    travel_alpha: float = 0.3,
    show_labels: bool = True,
    show_tw_bounds: bool = True,
    show_travel: bool = True,
    bar_height: float = 0.6,
    font_size: int = 8,
    speed: float = 1.0,
    save_path: Optional[str] = None,
    dpi: int = 150,
):
    """
    Plot a Gantt chart showing the schedule of visits with time windows.

    This visualization shows:
    - Each route as a horizontal row
    - Time windows as gray background bars
    - Service times as colored bars
    - Travel times as lighter connecting bars (optional)
    - Client IDs as labels

    Args:
        problem: The routing Problem object (must have Rendezvous attributes)
        solution: The Solution object to visualize
        ax: Optional matplotlib axes. If None, creates new figure
        figsize: Figure size as (width, height) tuple
        title: Plot title. If None, auto-generates
        route_colors: List of colors for routes. Uses default palette if None
        tw_color: Color for time window background bars
        tw_alpha: Transparency for time window bars (0-1)
        service_alpha: Transparency for service bars (0-1)
        travel_color: Color for travel time bars
        travel_alpha: Transparency for travel bars (0-1)
        show_labels: Show client ID labels on bars
        show_tw_bounds: Show time window boundaries as text
        show_travel: Show travel time bars between clients
        bar_height: Height of bars (0-1)
        font_size: Font size for labels
        speed: Travel speed (distance units per time unit), default 1.0
        save_path: If provided, saves the figure to this path
        dpi: DPI for saved figure

    Returns:
        matplotlib Figure and Axes objects (fig, ax)

    Example:
        >>> # For VRPTW problems
        >>> fig, ax = plot_gantt(problem, solution)
        >>> plt.show()

        >>> # Customize appearance
        >>> plot_gantt(problem, solution,
        ...            show_travel=True,
        ...            speed=1.0,
        ...            title="Vehicle Schedules")

    Note:
        Requires clients to have Rendezvous (time window) attributes.
        Optionally uses ServiceQuery for service times and GeoNode for
        travel time calculation.
    """
    try:
        import matplotlib.pyplot as plt
        import matplotlib.patches as mpatches
        from matplotlib.lines import Line2D
    except ImportError:
        raise ImportError(
            "matplotlib is required for visualization. "
            "Install it with: pip install matplotlib"
        )

    # Create figure if needed
    if ax is None:
        fig, ax = plt.subplots(figsize=figsize)
    else:
        fig = ax.get_figure()

    colors = route_colors or DEFAULT_COLORS

    # Build client data map
    client_data = {}
    for client in problem.get_clients():
        cid = client.get_id()
        client_data[cid] = {
            'x': client.x,
            'y': client.y,
            'tw_open': client.tw_open if client.tw_open is not None else 0,
            'tw_close': client.tw_close if client.tw_close is not None else float('inf'),
            'service_time': client.service_time if client.service_time is not None else 0,
        }

    # Get depot data
    depot_data = {'x': 0, 'y': 0, 'tw_open': 0, 'tw_close': float('inf')}
    depots = problem.get_depots()
    if depots:
        depot = depots[0]
        depot_data['x'] = depot.x if depot.x is not None else 0
        depot_data['y'] = depot.y if depot.y is not None else 0
        depot_data['tw_open'] = depot.tw_open if depot.tw_open is not None else 0
        depot_data['tw_close'] = depot.tw_close if depot.tw_close is not None else float('inf')

    # Calculate distance between two points
    def calc_distance(x1, y1, x2, y2):
        if x1 is None or y1 is None or x2 is None or y2 is None:
            return 0
        return math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2)

    # Process tours and build schedule
    tours = solution.get_tours() if solution else []
    y_labels = []
    max_time = 0

    for tour_idx, tour in enumerate(tours):
        client_ids = tour.get_client_ids()
        if not client_ids:
            continue

        color = colors[tour_idx % len(colors)]
        y_pos = len(tours) - tour_idx - 1  # Reverse order (route 1 at top)
        y_labels.append(f"Route {tour_idx + 1}")

        # Start from depot
        current_time = depot_data['tw_open']
        prev_x, prev_y = depot_data['x'], depot_data['y']

        for i, cid in enumerate(client_ids):
            if cid not in client_data:
                continue

            data = client_data[cid]
            tw_open = data['tw_open']
            tw_close = data['tw_close']
            service_time = data['service_time']
            x, y = data['x'], data['y']

            # Calculate travel time
            distance = calc_distance(prev_x, prev_y, x, y)
            travel_time = distance / speed if speed > 0 else 0

            # Draw travel bar
            if show_travel and travel_time > 0:
                ax.barh(y_pos, travel_time, left=current_time,
                        height=bar_height * 0.3, color=travel_color,
                        alpha=travel_alpha, edgecolor='none')

            # Arrival time (wait if early)
            arrival_time = current_time + travel_time
            service_start = max(arrival_time, tw_open)

            # Draw time window background
            tw_width = tw_close - tw_open
            if tw_width > 0 and tw_width < float('inf'):
                ax.barh(y_pos, tw_width, left=tw_open,
                        height=bar_height, color=tw_color,
                        alpha=tw_alpha, edgecolor='gray', linewidth=0.5)

                # Show TW bounds
                if show_tw_bounds:
                    ax.text(tw_open, y_pos + bar_height / 2 + 0.05,
                            f'{tw_open:.0f}', fontsize=font_size - 2,
                            ha='left', va='bottom', color='gray')
                    ax.text(tw_close, y_pos + bar_height / 2 + 0.05,
                            f'{tw_close:.0f}', fontsize=font_size - 2,
                            ha='right', va='bottom', color='gray')

            # Draw service bar
            if service_time > 0:
                ax.barh(y_pos, service_time, left=service_start,
                        height=bar_height, color=color,
                        alpha=service_alpha, edgecolor='white', linewidth=0.5)
            else:
                # Draw thin bar for zero service time
                ax.barh(y_pos, max(1, (tw_close - tw_open) * 0.05), left=service_start,
                        height=bar_height, color=color,
                        alpha=service_alpha, edgecolor='white', linewidth=0.5)

            # Add client label
            if show_labels:
                label_x = service_start + service_time / 2 if service_time > 0 else service_start + 0.5
                ax.text(label_x, y_pos, str(cid),
                        fontsize=font_size, ha='center', va='center',
                        color='white' if service_time > 2 else 'black',
                        fontweight='bold')

            # Update for next iteration
            current_time = service_start + service_time
            max_time = max(max_time, current_time, tw_close if tw_close < float('inf') else 0)
            prev_x, prev_y = x, y

        # Return to depot
        if show_travel:
            distance = calc_distance(prev_x, prev_y, depot_data['x'], depot_data['y'])
            travel_time = distance / speed if speed > 0 else 0
            if travel_time > 0:
                ax.barh(y_pos, travel_time, left=current_time,
                        height=bar_height * 0.3, color=travel_color,
                        alpha=travel_alpha, edgecolor='none')
                max_time = max(max_time, current_time + travel_time)

    # Set up axes
    ax.set_yticks(range(len(y_labels)))
    ax.set_yticklabels(reversed(y_labels))
    ax.set_xlabel('Time', fontsize=10)
    ax.set_ylabel('Vehicle', fontsize=10)

    # Set x-axis limits with some padding
    ax.set_xlim(0, max_time * 1.05)
    ax.set_ylim(-0.5, len(y_labels) - 0.5)

    # Grid
    ax.grid(True, axis='x', alpha=0.3)
    ax.axvline(x=0, color='black', linewidth=1)

    # Title
    if title is None:
        title = f"Schedule: {len(tours)} routes"
        if solution:
            title += f", cost={solution.cost:.2f}"
    ax.set_title(title, fontsize=12, fontweight='bold')

    # Legend
    legend_elements = [
        mpatches.Patch(facecolor=tw_color, alpha=tw_alpha,
                       edgecolor='gray', label='Time Window'),
        mpatches.Patch(facecolor=colors[0], alpha=service_alpha,
                       label='Service'),
    ]
    if show_travel:
        legend_elements.append(
            mpatches.Patch(facecolor=travel_color, alpha=travel_alpha,
                           label='Travel')
        )
    ax.legend(handles=legend_elements, loc='upper right', fontsize=font_size)

    fig.tight_layout()

    # Save if requested
    if save_path:
        fig.savefig(save_path, dpi=dpi, bbox_inches='tight')

    return fig, ax


__all__ = [
    'plot_solution',
    'plot_problem',
    'plot_gantt',
    'plot_convergence',
    'create_animation',
    'compare_solutions',
    'DEFAULT_COLORS',
]
